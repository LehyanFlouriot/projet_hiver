#include "EGA_MeleeBase.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"

UEGA_MeleeBase::UEGA_MeleeBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.Attack"));

	AbilityTags.AddTag(AttackTag);
	ActivationOwnedTags.AddTag(AttackTag);

	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Jump")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Interact")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Potion")));

	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsGrounded")));
}

void UEGA_MeleeBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
{
	OnGameplayAbilityEndedWithData.AddLambda([&](const FAbilityEndedData &EndedData)
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(SlowEffectHandle);

		if (IsValid(MontageTask)) MontageTask->EndTask();
		if (IsValid(GameplayEventTask)) GameplayEventTask->EndTask();

		if (EndedData.bWasCancelled)
			Character->ResetMeleeHitTargets();
	});

	CommitAbility(Handle, ActorInfo, ActivationInfo);

	SlowEffectHandle = ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,
		MakeOutgoingGameplayEffectSpec(SlowEffect));
	
	Character = CastChecked<APlayerCharacter>(CurrentActorInfo->OwnerActor);

	Character->SetMeleeSphereRadius(SphereInfo.SphereRadius);
	Character->SetMeleeSphereOffset(SphereInfo.SphereOffset);

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage, MontagePlayingRate);
	MontageTask->OnCompleted.AddDynamic(this, &UEGA_MeleeBase::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UEGA_MeleeBase::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UEGA_MeleeBase::OnMontageCompleted);

	GameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.MeleeStrike")));
	GameplayEventTask->EventReceived.AddDynamic(this, &UEGA_MeleeBase::OnGameplayEventReceived);

	MontageTask->ReadyForActivation();
	GameplayEventTask->ReadyForActivation();
}

void UEGA_MeleeBase::OnMontageCompleted()
{
	if (HasAuthority(&CurrentActivationInfo))
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, !Character->GetEndMeleeNormally());
}

void UEGA_MeleeBase::OnGameplayEventReceived(FGameplayEventData GameplayEventData)
{
	ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, CreateDamageEffect(DamageEffect, DamageInfo), GameplayEventData.TargetData);
}
