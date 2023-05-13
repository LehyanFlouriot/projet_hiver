#include "EGA_Block.h"

#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "ProjetHiver/Characters/BaseCharacter.h"
#include "ProjetHiver/Characters/BaseNPC.h"

UEGA_Block::UEGA_Block()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FGameplayTag BlockTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.Move.Block"));

	AbilityTags.AddTag(BlockTag);
	ActivationOwnedTags.AddTag(BlockTag);

	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Move.Block")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Interact")));

	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsGrounded")));

	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.Attack")));
}

void UEGA_Block::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	OnGameplayAbilityEnded.AddLambda([&](UGameplayAbility* GameplayAbility)
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(BlockingEffectHandle);

		if (bShouldRemoveParryingEffect)
			BP_RemoveGameplayEffectFromOwnerWithHandle(ParryingEffectHandle);

		if (IsValid(ParryingDelayTask)) ParryingDelayTask->EndTask();
		if (IsValid(ParryEventTask)) ParryEventTask->EndTask();
		if (IsValid(InputReleaseTask)) InputReleaseTask->EndTask();

		MontageStop();
	});

	if (!CommitCheck(Handle, ActorInfo, ActivationInfo)) EndAbility(Handle, ActorInfo, ActivationInfo, true, true);

	InitCooldownOnWidget();

	BlockingEffectHandle = ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,
	                                                      MakeOutgoingGameplayEffectSpec(BlockingEffect));

	ParryingEffectHandle = ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,
														  MakeOutgoingGameplayEffectSpec(ParryingEffect));

	bShouldRemoveParryingEffect = true;

	ParryingDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, ParryingLength);
	ParryingDelayTask->OnFinish.AddDynamic(this, &UEGA_Block::OnDelayFinish);

	ParryEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag("Character.AbilityEvent.Parry"));
	ParryEventTask->EventReceived.AddDynamic(this, &UEGA_Block::OnParryEventReceived);

	InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	InputReleaseTask->OnRelease.AddDynamic(this, &UEGA_Block::OnRelease);

	ParryingDelayTask->ReadyForActivation();
	ParryEventTask->ReadyForActivation();
	InputReleaseTask->ReadyForActivation();
}

void UEGA_Block::OnDelayFinish()
{
	if (IsValid(ParryEventTask)) ParryEventTask->EndTask();

	BP_RemoveGameplayEffectFromOwnerWithHandle(ParryingEffectHandle);
	bShouldRemoveParryingEffect = false;
}

void UEGA_Block::OnParryEventReceived(FGameplayEventData EventData)
{
	if (EventData.Target->IsA(ABaseNPC::StaticClass()))
	{
		FGameplayEffectSpecHandle GESpecHandle = MakeOutgoingGameplayEffectSpec(StunEffect, GetAbilityLevel());
		GESpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("AbilityData.StunLength")), StunLength);
		ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, GESpecHandle, EventData.TargetData);
	}
}

void UEGA_Block::OnRelease(float TimeHeld)
{
	CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);

	if (HasAuthority(&CurrentActivationInfo))
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
