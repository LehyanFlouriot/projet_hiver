#include "EGA_Jump.h"

#include "ProjetHiver/Characters/BaseCharacter.h"

UEGA_Jump::UEGA_Jump()
{
	AbilityInputID = EGASAbilityInputID::JumpAction;

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Jump")));

	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsGrounded")));

	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Move.Dodge")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Interact")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Potion")));

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsAiming")));

	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.Attack")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Move.Block")));
}

void UEGA_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	ABaseCharacter* Character = CastChecked<ABaseCharacter>(ActorInfo->OwnerActor.Get());
	Character->Jump();
	CommitAbility(Handle, ActorInfo, ActivationInfo);

	if (HasAuthority(&CurrentActivationInfo))
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
