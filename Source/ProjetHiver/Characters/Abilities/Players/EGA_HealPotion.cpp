#include "EGA_HealPotion.h"

#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjetHiver/BDD/UserScoreboardStatistic.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/Characters/AttributeSets/HealthAttributeSet.h"
#include "ProjetHiver/Characters/AttributeSets/PotionAttributeSet.h"
#include "ProjetHiver/Contracts/ContractStatus.h"
#include "ProjetHiver/Contracts/FailableContractType.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerState.h"
#include "ProjetHiver/Items/ReplicatedStaticMeshActor.h"

UEGA_HealPotion::UEGA_HealPotion()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    const FGameplayTag PotionTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Potion"));

    AbilityTags.AddTag(PotionTag);
    ActivationOwnedTags.AddTag(PotionTag);

    BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action")));

    ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsAiming")));

    ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsGrounded")));

	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Move.Block")));
}

void UEGA_HealPotion::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
    OnGameplayAbilityEnded.AddLambda([&](UGameplayAbility* GameplayAbility)
    {
        if (IsValid(SyncTask)) SyncTask->EndTask();
		if (IsValid(MontageTask)) MontageTask->EndTask();
		if (IsValid(GameplayEventTask)) GameplayEventTask->EndTask();

        if (HasAuthority(&CurrentActivationInfo))
        {
            Character->SetLeftWeaponVisibility(false);
            PotionBottleActor->Destroy();
        }
    });

    if (!CommitCheck(Handle, ActorInfo, ActivationInfo))
        if (HasAuthority(&CurrentActivationInfo))
            CancelAbility(Handle, ActorInfo, ActivationInfo, true);

    Character = CastChecked<APlayerCharacter>(ActorInfo->OwnerActor);

    SyncTask = UAbilityTask_NetworkSyncPoint::WaitNetSync(this, EAbilityTaskNetSyncType::OnlyServerWait);
    SyncTask->OnSync.AddDynamic(this, &UEGA_HealPotion::OnSync);

    MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, DrinkingMontage, MontagePlayingRate);
	MontageTask->OnCompleted.AddDynamic(this, &UEGA_HealPotion::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UEGA_HealPotion::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UEGA_HealPotion::OnMontageCompleted);

	GameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.DrinkPotion")));
	GameplayEventTask->EventReceived.AddDynamic(this, &UEGA_HealPotion::OnGameplayEventReceived);

    if (HasAuthority(&ActivationInfo))
    {
        Character->SetLeftWeaponVisibility(true);

        const FName SocketName = FName("PotionSocket");
        const auto Transform = Character->GetMesh()->GetSocketTransform(SocketName);

    	PotionBottleActor = CastChecked<AEfhorisGameMode>(GetWorld()->GetAuthGameMode())->SpawnActor<AReplicatedStaticMeshActor>(PotionBottle.Get(), Transform);
        PotionBottleActor->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), SocketName);
    }
    
	MontageTask->ReadyForActivation();
    GameplayEventTask->ReadyForActivation();
}

bool UEGA_HealPotion::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    const APlayerCharacter* PlayerCharacter = CastChecked<APlayerCharacter>(ActorInfo->OwnerActor);

    return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) &&
			PlayerCharacter->GetPotionCharge() > 0.f &&
			PlayerCharacter->GetHealth() < PlayerCharacter->GetMaxHealth();
}

void UEGA_HealPotion::OnSync()
{
    CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);

    FGameplayEffectSpecHandle GESpecHandle = MakeOutgoingGameplayEffectSpec(HealEffect, GetAbilityLevel());
    const float MaxHealth = GetAbilitySystemComponentFromActorInfo()->GetNumericAttribute(UHealthAttributeSet::GetMaxHealthAttribute());
    GESpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Character.Statistics.Health")), MaxHealth * HealCoefficient);
    ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, GESpecHandle);

    if (HasAuthority(&CurrentActivationInfo))
    {
	    if (const AEfhorisGameState* GameState = GetWorld()->GetGameState<AEfhorisGameState>(); IsValid(GameState))
		{
            AFailableContract* Contract = GameState->GetFailableContract();

            if (Contract->GetType() == EFailableContractType::Potion && Contract->GetStatus() == EContractStatus::OnGoing)
                Contract->IncreaseCurrentValue();
		}

        if (const APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwningActorFromActorInfo()); IsValid(Player))
			if (AEfhorisPlayerState* PlayerState = Player->GetPlayerState<AEfhorisPlayerState>(); IsValid(PlayerState))
				PlayerState->IncreaseStatistic(EUserScoreboardStatistic::PotionsDrank, 1);
    }
}

void UEGA_HealPotion::OnMontageCompleted()
{
    if (HasAuthority(&CurrentActivationInfo))
    	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UEGA_HealPotion::OnGameplayEventReceived(FGameplayEventData GameplayEventData)
{
    SyncTask->ReadyForActivation();
}
