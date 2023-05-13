#include "EGA_MultiHitMeleeBase.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "ProjetHiver/Characters/BaseCharacter.h"
#include "ProjetHiver/Characters/BaseNPC.h"

UEGA_MultiHitMeleeBase::UEGA_MultiHitMeleeBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action")));

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsAiming")));

	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsGrounded")));
}

void UEGA_MultiHitMeleeBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
{
	OnGameplayAbilityEnded.AddLambda([&](UGameplayAbility* GameplayAbility)
	{
		if (IsValid(MontageTask)) MontageTask->EndTask();
		if (IsValid(MeleeStrikeEventTask)) MeleeStrikeEventTask->EndTask();
		if (IsValid(ProgressMultiHitEventTask)) ProgressMultiHitEventTask->EndTask();
		if (IsValid(TPBackEventTask)) TPBackEventTask->EndTask();
	});

	if (!CommitCheck(Handle, ActorInfo, ActivationInfo)) EndAbility(Handle, ActorInfo, ActivationInfo, true, true);

	InitCooldownOnWidget();

	AbilityProgress = 0;

	ABaseCharacter* Character = CastChecked<ABaseCharacter>(CurrentActorInfo->OwnerActor);
	Character->SetMeleeSphereRadius(SphereInfos[AbilityProgress].SphereRadius);
	Character->SetMeleeSphereOffset(SphereInfos[AbilityProgress].SphereOffset);

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage, MontagePlayingRate);
	MontageTask->OnCompleted.AddDynamic(this, &UEGA_MultiHitMeleeBase::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UEGA_MultiHitMeleeBase::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UEGA_MultiHitMeleeBase::OnMontageCompleted);

	MeleeStrikeEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.MeleeStrike")));
	MeleeStrikeEventTask->EventReceived.AddDynamic(this, &UEGA_MultiHitMeleeBase::OnMeleeStrikeEventReceived);

	ProgressMultiHitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.ProgressMultiHit")));
	ProgressMultiHitEventTask->EventReceived.AddDynamic(this, &UEGA_MultiHitMeleeBase::OnProgressMultiHitEventReceived);

	if (bShouldTP)
	{
		TPBackEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.TPBack")));
		TPBackEventTask->EventReceived.AddDynamic(this, &UEGA_MultiHitMeleeBase::OnTPBackEventReceived);
	}

	MontageTask->ReadyForActivation();
	MeleeStrikeEventTask->ReadyForActivation();
	ProgressMultiHitEventTask->ReadyForActivation();
	if(bShouldTP)
		TPBackEventTask->ReadyForActivation();
}

bool UEGA_MultiHitMeleeBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (DamageInfos.Num() != SphereInfos.Num()) return false;
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UEGA_MultiHitMeleeBase::OnMontageCompleted()
{
	CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);

	if (HasAuthority(&CurrentActivationInfo))
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UEGA_MultiHitMeleeBase::OnMeleeStrikeEventReceived(FGameplayEventData GameplayEventData)
{
	ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, CreateDamageEffect(DamageEffect, DamageInfos[AbilityProgress]), GameplayEventData.TargetData);
}

void UEGA_MultiHitMeleeBase::OnProgressMultiHitEventReceived(FGameplayEventData GameplayEventData)
{
	AbilityProgress = FMath::Min(AbilityProgress + 1, DamageInfos.Num());
}

void UEGA_MultiHitMeleeBase::OnTPBackEventReceived(FGameplayEventData GameplayEventData)
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		ABaseCharacter* Character = CastChecked<ABaseCharacter>(CurrentActorInfo->OwnerActor);
		ABaseNPC* NPC = Cast<ABaseNPC>(Character);
		const ABaseCharacter* Target = NPC->GetTarget();

		const FVector TargetBackward = Target->GetActorForwardVector() * -1;

		const FVector Position = Target->GetActorLocation();
		const FVector BackPosition = Position + TargetBackward * TPOffset;
		//NPC->SetActorLocation(Position);
		//NPC->SetActorRotation(Target->GetActorRotation());
		if (!NPC->TeleportTo(BackPosition, Target->GetActorRotation()))
			CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}
