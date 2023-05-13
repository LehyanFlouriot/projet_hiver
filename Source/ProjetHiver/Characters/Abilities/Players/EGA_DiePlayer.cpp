#include "EGA_DiePlayer.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpectatorPawn.h"
#include "ProjetHiver/AI/AIManager.h"
#include "ProjetHiver/AI/Signals/Signal.h"
#include "ProjetHiver/AI/Signals/SignalComponent.h"
#include "ProjetHiver/BDD/UserScoreboardStatistic.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/Characters/Mercenary.h"
#include "ProjetHiver/Characters/AttributeSets/HealthAttributeSet.h"
#include "ProjetHiver/Contracts/ContractStatus.h"
#include "ProjetHiver/Contracts/FailableContractType.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerState.h"
#include "ProjetHiver/GAS/EfhorisAbilitySystemComponent.h"

UEGA_DiePlayer::UEGA_DiePlayer()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FGameplayTag DeathTag = FGameplayTag::RequestGameplayTag(FName("Character.Die"));

	AbilityTags.AddTag(DeathTag);
	ActivationOwnedTags.AddTag(DeathTag);

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsDying")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsDead")));

	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action")));
}

void UEGA_DiePlayer::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	OnGameplayAbilityEndedWithData.AddLambda([&](const FAbilityEndedData& EndedData)
	{
		if (IsValid(KnockdownMontageTask)) KnockdownMontageTask->EndTask();
		if (IsValid(GetupMontageTask)) GetupMontageTask->EndTask();

		if (IsValid(StartInteractEventTask)) StartInteractEventTask->EndTask();
		if (IsValid(StopInteractEventTask)) StopInteractEventTask->EndTask();
		if (IsValid(WaitForReviveEventTask)) WaitForReviveEventTask->EndTask();

		if (IsValid(DyingDelayTask)) DyingDelayTask->EndTask();
		if (IsValid(ReviveDelayTask)) StopInteractEventTask->EndTask();

		bIsInteractedWith = false;
		bShouldDieOnRelease = false;

		if (EndedData.bWasCancelled)
		{
			if (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(DyingTag))
			{
				GetAbilitySystemComponentFromActorInfo()->RemoveReplicatedLooseGameplayTag(DyingTag);
				GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(DyingTag);
			}

			if (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(DeadTag))
			{
				GetAbilitySystemComponentFromActorInfo()->RemoveReplicatedLooseGameplayTag(DeadTag);
				GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(DeadTag);
			}

			if (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(InvincibleTag))
			{
				GetAbilitySystemComponentFromActorInfo()->RemoveReplicatedLooseGameplayTag(InvincibleTag);
				GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(InvincibleTag);
			}

			if (HasAuthority(&CurrentActivationInfo))
				SetUserToWalkingMode();
		}
	});

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		if (HasAuthority(&CurrentActivationInfo))
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);

	CastChecked<ABaseCharacter>(ActorInfo->OwnerActor)->GetAbilitySystemComponent()->CancelAllAbilities(this);

	if (HasAuthority(&ActivationInfo))
		SetUserToFlyingMode();

	KnockdownMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, KnockdownMontage, KnockdownMontagePlayingRate);
	KnockdownMontageTask->OnBlendOut.AddDynamic(this, &UEGA_DiePlayer::OnKnockdownMontageCompleted);
	KnockdownMontageTask->ReadyForActivation();
}

void UEGA_DiePlayer::OnKnockdownMontageCompleted()
{
	WaitForReviveEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.Revive")));
	WaitForReviveEventTask->EventReceived.AddDynamic(this, &UEGA_DiePlayer::OnWaitForReviveReceived);
	WaitForReviveEventTask->ReadyForActivation();

	if (HasAuthority(&CurrentActivationInfo))
	{
		GetAbilitySystemComponentFromActorInfo()->AddReplicatedLooseGameplayTag(DyingTag);
		GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(DyingTag);

		DyingDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, DyingTime);
		DyingDelayTask->OnFinish.AddDynamic(this, &UEGA_DiePlayer::OnDyingDelayFinished);
		DyingDelayTask->ReadyForActivation();

		StartInteractEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Interact.Start")));
		StartInteractEventTask->EventReceived.AddDynamic(this, &UEGA_DiePlayer::OnStartInteractReceived);
		StartInteractEventTask->ReadyForActivation();

		StopInteractEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Interact.Stop")));
		StopInteractEventTask->EventReceived.AddDynamic(this, &UEGA_DiePlayer::OnStopInteractReceived);
		StopInteractEventTask->ReadyForActivation();

		if (APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwningActorFromActorInfo()); IsValid(Player))
		{
			SetUserToFallingMode();
			
			USignal* Signal = NewObject<USignal>();
			Signal->Init(ESignalType::PlayerDied, 3000.f, Player);
			Player->GetSignalComponent()->SendSignal(Signal);

			if (AEfhorisPlayerState* PlayerState = Player->GetPlayerState<AEfhorisPlayerState>(); IsValid(PlayerState))
				PlayerState->IncreaseStatistic(EUserScoreboardStatistic::Knockouts, 1);
		}

		if (AEfhorisGameMode* GameMode = GetWorld()->GetAuthGameMode<AEfhorisGameMode>())
			GameMode->TryLoseGame();
	}
}

void UEGA_DiePlayer::OnGetupMontageCompleted()
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		GetAbilitySystemComponentFromActorInfo()->RemoveReplicatedLooseGameplayTag(InvincibleTag);
		GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(InvincibleTag);

		SetUserToWalkingMode();
	}

	if (HasAuthority(&CurrentActivationInfo))
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UEGA_DiePlayer::OnDyingDelayFinished()
{
	if (bIsInteractedWith)
	{
		bShouldDieOnRelease = true;
		return;
	}

	if (HasAuthority(&CurrentActivationInfo))
	{
		GetAbilitySystemComponentFromActorInfo()->RemoveReplicatedLooseGameplayTag(DyingTag);
		GetAbilitySystemComponentFromActorInfo()->AddReplicatedLooseGameplayTag(DeadTag);

		GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(DyingTag);
		GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(DeadTag);

		if (APlayerCharacter* Player = Cast<APlayerCharacter>(CurrentActorInfo->OwnerActor); IsValid(Player))
		{
			//Remove this player from the list of players to revive
			if (AAIManager* Manager = GetWorld()->GetGameState<AEfhorisGameState>()->GetAIManager(); IsValid(Manager)) {
				for (AMercenary* Mercenary : Manager->GetMercenaries()) {
					Mercenary->DeletePlayerToRevive(Player);
				}
			}
			//Let hired mercenaries find a new master
			Player->MercenariesFindNewMaster();

			AEfhorisGameMode* GameMode = GetWorld()->GetAuthGameMode<AEfhorisGameMode>();
			GameMode->AddPawnToDeleteOnLevelReset(Player);

			ASpectatorPawn* Spectator = GameMode->SpawnActor<ASpectatorPawn>(GameMode->SpectatorClass, Player->GetFollowCamera()->GetComponentTransform());
			Player->GetController()->Possess(Spectator);

			if (AEfhorisPlayerState* PlayerState = Player->GetPlayerState<AEfhorisPlayerState>(); IsValid(PlayerState))
				PlayerState->IncreaseStatistic(EUserScoreboardStatistic::Deaths, 1);

			GameMode->TryLoseGame();
		}
	}

	if (HasAuthority(&CurrentActivationInfo))
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UEGA_DiePlayer::OnReviveDelayFinished()
{
	// We want to be sure that death won't happen on a weird timing.
	if (IsValid(DyingDelayTask)) DyingDelayTask->EndTask();

	if (APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwningActorFromActorInfo()); IsValid(Player))
	{
		Player->Multicast_SendReviveEvent();

		SetUserToNoMoveMode();

		if (AEfhorisPlayerState* PlayerState = Player->GetPlayerState<AEfhorisPlayerState>(); IsValid(PlayerState))
			PlayerState->IncreaseStatistic(EUserScoreboardStatistic::Resurrections, 1);

		//Remove this player from the list of players to revive
		if (AAIManager* Manager = GetWorld()->GetGameState<AEfhorisGameState>()->GetAIManager(); IsValid(Manager)) {
			for (AMercenary* Mercenary : Manager->GetMercenaries()) {
				Mercenary->DeletePlayerToRevive(Player);
			}
		}
	}
}

void UEGA_DiePlayer::OnStartInteractReceived(FGameplayEventData GameplayEventData)
{
	if (bIsInteractedWith) return;

	bIsInteractedWith = true;

	if (IsValid(ReviveDelayTask)) ReviveDelayTask->EndTask();

	ReviveDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, ReviveTime);
	ReviveDelayTask->OnFinish.AddDynamic(this, &UEGA_DiePlayer::OnReviveDelayFinished);
	ReviveDelayTask->ReadyForActivation();
}

void UEGA_DiePlayer::OnStopInteractReceived(FGameplayEventData GameplayEventData)
{
	if (!bIsInteractedWith) return;

	bIsInteractedWith = false;
	if (IsValid(ReviveDelayTask)) ReviveDelayTask->EndTask();

	if (bShouldDieOnRelease) OnDyingDelayFinished();
}

void UEGA_DiePlayer::OnWaitForReviveReceived(FGameplayEventData GameplayEventData)
{
	GetupMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, GetupMontage, GetupMontagePlayingRate, NAME_None, true);

	if (HasAuthority(&CurrentActivationInfo))
	{
		GetAbilitySystemComponentFromActorInfo()->AddReplicatedLooseGameplayTag(InvincibleTag);
		GetAbilitySystemComponentFromActorInfo()->RemoveReplicatedLooseGameplayTag(DyingTag);

		GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(InvincibleTag);
		GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(DyingTag);

		FGameplayEffectSpecHandle GESpecHandle = MakeOutgoingGameplayEffectSpec(HealEffect, GetAbilityLevel());
		const float MaxHealth = GetAbilitySystemComponentFromActorInfo()->GetNumericAttribute(UHealthAttributeSet::GetMaxHealthAttribute());
		GESpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Character.Statistics.Health")), MaxHealth * HealCoefficient);
		ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, GESpecHandle);

		GetupMontageTask->OnBlendOut.AddDynamic(this, &UEGA_DiePlayer::OnGetupMontageCompleted);

		if (const AEfhorisGameState* GameState = GetWorld()->GetGameState<AEfhorisGameState>(); IsValid(GameState))
		{
			AFailableContract* Contract = GameState->GetFailableContract();

			if (Contract->GetType() == EFailableContractType::Resurrect && Contract->GetStatus() == EContractStatus::OnGoing)
				Contract->IncreaseCurrentValue();
		}
	}

	GetupMontageTask->ReadyForActivation();
}
