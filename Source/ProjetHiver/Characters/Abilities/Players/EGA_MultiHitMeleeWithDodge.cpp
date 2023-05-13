#include "EGA_MultiHitMeleeWithDodge.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

void UEGA_MultiHitMeleeWithDodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (HasAuthority(&ActivationInfo))
	{
		OnGameplayAbilityEnded.AddLambda([&](UGameplayAbility* GameplayAbility)
		{
			if (IsValid(InvincibilityBeginEventTask)) InvincibilityBeginEventTask->EndTask();
			if (IsValid(InvincibilityEndEventTask)) InvincibilityEndEventTask->EndTask();

			SetUserToWalkingMode();

			if (HasAuthority(&CurrentActivationInfo))
				BP_RemoveGameplayEffectFromOwnerWithHandle(InvincibleEffectHandle);
		});

		if (!CommitCheck(Handle, ActorInfo, ActivationInfo)) CancelAbility(Handle, ActorInfo, ActivationInfo, true);

		InitCooldownOnWidget();

		InvincibilityBeginEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag("Character.Animation.InvincibilityBegin"));
		InvincibilityBeginEventTask->EventReceived.AddDynamic(this, &UEGA_MultiHitMeleeWithDodge::OnInvincibilityBeginEventReceived);

		InvincibilityEndEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag("Character.Animation.InvincibilityEnd"));
		InvincibilityEndEventTask->EventReceived.AddDynamic(this, &UEGA_MultiHitMeleeWithDodge::OnInvincibilityEndEventReceived);

		InvincibilityBeginEventTask->ReadyForActivation();

		SetUserToFlyingMode();
	}
}

void UEGA_MultiHitMeleeWithDodge::OnInvincibilityBeginEventReceived(FGameplayEventData EventData)
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		FGameplayEffectSpecHandle GESpecHandle = MakeOutgoingGameplayEffectSpec(InvincibleEffect, GetAbilityLevel());
		InvincibleEffectHandle = ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, GESpecHandle);

		InvincibilityEndEventTask->ReadyForActivation();
	}
}

void UEGA_MultiHitMeleeWithDodge::OnInvincibilityEndEventReceived(FGameplayEventData EventData)
{
	if (HasAuthority(&CurrentActivationInfo))
		BP_RemoveGameplayEffectFromOwnerWithHandle(InvincibleEffectHandle);
}
