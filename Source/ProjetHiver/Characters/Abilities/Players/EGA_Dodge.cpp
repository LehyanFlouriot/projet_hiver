#include "EGA_Dodge.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"

UEGA_Dodge::UEGA_Dodge()
{
	AbilityInputID = EGASAbilityInputID::DodgeAction;

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FGameplayTag DodgeTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.Move.Dodge"));

	AbilityTags.AddTag(DodgeTag);
	ActivationOwnedTags.AddTag(DodgeTag);

	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action")));

	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsGrounded")));

	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Move.Block")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.Attack")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.RangeAttack")));
}

void UEGA_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	OnGameplayAbilityEnded.AddLambda([&](UGameplayAbility* GameplayAbility)
	{
		if (IsValid(MontageTask)) MontageTask->EndTask();
		if (IsValid(InvincibilityBeginEventTask)) InvincibilityBeginEventTask->EndTask();
		if (IsValid(InvincibilityEndEventTask)) InvincibilityEndEventTask->EndTask();

		if (HasAuthority(&CurrentActivationInfo))
			BP_RemoveGameplayEffectFromOwnerWithHandle(InvincibleEffectHandle);
	});

	if (!CommitCheck(Handle, ActorInfo, ActivationInfo)) EndAbility(Handle, ActorInfo, ActivationInfo, true, true);

	InitCooldownOnWidget();

	const FVector2D MovementVector = CastChecked<APlayerCharacter>(CurrentActorInfo->OwnerActor)->GetLastInputVector();

	if (MovementVector.X > 0.f)
		MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, DodgeMontageR, MontagePlayingRate);
	else if (MovementVector.X < 0.f)
		MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, DodgeMontageL, MontagePlayingRate);
	else if (MovementVector.Y > 0.f)
		MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, DodgeMontageF, MontagePlayingRate);
	else
		MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, DodgeMontageB, MontagePlayingRate);

	MontageTask->OnCompleted.AddDynamic(this, &UEGA_Dodge::OnCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UEGA_Dodge::OnCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UEGA_Dodge::OnCompleted);

	MontageTask->ReadyForActivation();

	if (HasAuthority(&ActivationInfo))
	{
		InvincibilityBeginEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag("Character.Animation.InvincibilityBegin"));
		InvincibilityBeginEventTask->EventReceived.AddDynamic(this, &UEGA_Dodge::OnInvincibilityBeginEventReceived);

		InvincibilityEndEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag("Character.Animation.InvincibilityEnd"));
		InvincibilityEndEventTask->EventReceived.AddDynamic(this, &UEGA_Dodge::OnInvincibilityEndEventReceived);

		InvincibilityBeginEventTask->ReadyForActivation();
	}
}

void UEGA_Dodge::OnCompleted()
{
	CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);

	if (HasAuthority(&CurrentActivationInfo))
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

}

void UEGA_Dodge::OnInvincibilityBeginEventReceived(FGameplayEventData EventData)
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		FGameplayEffectSpecHandle GESpecHandle = MakeOutgoingGameplayEffectSpec(InvincibleEffect, GetAbilityLevel());
		InvincibleEffectHandle = ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, GESpecHandle);

		InvincibilityEndEventTask->ReadyForActivation();
	}
}

void UEGA_Dodge::OnInvincibilityEndEventReceived(FGameplayEventData EventData)
{
	if (HasAuthority(&CurrentActivationInfo))
		BP_RemoveGameplayEffectFromOwnerWithHandle(InvincibleEffectHandle);
}
