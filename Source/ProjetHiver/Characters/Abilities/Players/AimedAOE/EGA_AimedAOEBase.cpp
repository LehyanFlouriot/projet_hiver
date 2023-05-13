#include "EGA_AimedAOEBase.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTargetActor_GroundTrace.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "GameplayAbilities/Public/AbilitySystemBlueprintLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProjetHiver/Characters/BaseCharacter.h"

UEGA_AimedAOEBase::UEGA_AimedAOEBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsGrounded")));
}

void UEGA_AimedAOEBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
										const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
										const FGameplayEventData* TriggerEventData)
{
	OnGameplayAbilityEnded.AddLambda([&](UGameplayAbility* GameplayAbility)
	{
		if (IsValid(StartMontageTask)) StartMontageTask->EndTask();
		if (IsValid(LoopMontageTask)) LoopMontageTask->EndTask();
		if (IsValid(EndMontageTask)) EndMontageTask->EndTask();
		if (IsValid(WaitDelayBeforeIsAimingRemovalTask)) WaitDelayBeforeIsAimingRemovalTask->EndTask();
		if (IsValid(WaitTargetDataEventTask)) WaitTargetDataEventTask->EndTask();

		if (IsAimingEffectHandle.IsValid())
			BP_RemoveGameplayEffectFromOwnerWithHandle(IsAimingEffectHandle);
	});

	if (!CommitCheck(Handle, ActorInfo, ActivationInfo)) CancelAbility(Handle, ActorInfo, ActivationInfo, true);

	InitCooldownOnWidget();

	bShouldEndAbility = false;

	IsAimingEffectHandle = ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,
		MakeOutgoingGameplayEffectSpec(IsAimingEffect));

	StartMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, StartAimingMontage, StartAnimationPlayingRate);
	StartMontageTask->OnBlendOut.AddDynamic(this, &UEGA_AimedAOEBase::OnStartMontageBlendOut);
	StartMontageTask->ReadyForActivation();

	WaitDelayBeforeIsAimingRemovalTask = UAbilityTask_WaitDelay::WaitDelay(this, DelayAfterConfirmOrCancel);
	WaitDelayBeforeIsAimingRemovalTask->OnFinish.AddDynamic(this, &UEGA_AimedAOEBase::OnWaitDelayBeforeIsAimingRemovalTask);
}

void UEGA_AimedAOEBase::OnStartMontageBlendOut()
{
	LoopMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AimingLoopMontage, LoopingAnimationPlayingRate);
	LoopMontageTask->ReadyForActivation();

	ABaseCharacter* Player = CastChecked<ABaseCharacter>(GetAvatarActorFromActorInfo());

	const FVector TargetLocation = MakeTargetLocationInfoFromOwnerActor().LiteralTransform.GetLocation() + Player->GetActorForwardVector() * AimingOffset;
	FGameplayAbilityTargetingLocationInfo TargetingInfo{ MakeTargetLocationInfoFromOwnerActor() };
	TargetingInfo.LiteralTransform.SetLocation(TargetLocation);

	AGameplayAbilityTargetActor_GroundTrace* TargetActor = CastChecked<AGameplayAbilityTargetActor_GroundTrace>(GetWorld()->SpawnActorDeferred<AGameplayAbilityTargetActor_GroundTrace>(TargetActorClass, FTransform{}, GetOwningActorFromActorInfo(), Player, ESpawnActorCollisionHandlingMethod::AlwaysSpawn));
	TargetActor->StartLocation = TargetingInfo;
	TargetActor->MaxRange = MaxRange;
	TargetActor->bTraceAffectsAimPitch = true;
	TargetActor->TraceProfile = FCollisionProfileName{ FName{ "TargetActorGroundLocation" } };
	TargetActor->FinishSpawning(FTransform{});

	WaitTargetDataEventTask = UAbilityTask_WaitTargetData::WaitTargetDataUsingActor(this, NAME_None, EGameplayTargetingConfirmation::UserConfirmed, TargetActor);
	WaitTargetDataEventTask->ValidData.AddDynamic(this, &UEGA_AimedAOEBase::OnValidDataReceived);
	WaitTargetDataEventTask->Cancelled.AddDynamic(this, &UEGA_AimedAOEBase::OnCancelledReceived);
	WaitTargetDataEventTask->ReadyForActivation();
}

void UEGA_AimedAOEBase::OnValidDataReceived(const FGameplayAbilityTargetDataHandle& Data)
{
	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		bShouldEndAbility = true;
		WaitDelayBeforeIsAimingRemovalTask->ReadyForActivation();
	}
	else
	{
		EndMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, EndAimingMontage, EndAnimationPlayingRate, NAME_None, false);
		EndMontageTask->ReadyForActivation();

		AoELocation = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(Data, 0).Location + SphereInfo.SphereOffset;

		ExecuteAOE(Data);

		WaitDelayBeforeIsAimingRemovalTask->ReadyForActivation();
	}
}

void UEGA_AimedAOEBase::OnCancelledReceived(const FGameplayAbilityTargetDataHandle& Data)
{
	MontageStop();
	CancelCooldownOnWidget();
	bShouldEndAbility = true;
	WaitDelayBeforeIsAimingRemovalTask->ReadyForActivation();
}

void UEGA_AimedAOEBase::OnWaitDelayBeforeIsAimingRemovalTask()
{
	BP_RemoveGameplayEffectFromOwnerWithHandle(IsAimingEffectHandle);

	if (HasAuthority(&CurrentActivationInfo) && bShouldEndAbility)
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
