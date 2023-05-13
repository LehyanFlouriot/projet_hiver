#include "EGA_AIMeleeBase.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "ProjetHiver/AI/EfhorisAIController.h"
#include "ProjetHiver/Characters/BaseNPC.h"

UEGA_AIMeleeBase::UEGA_AIMeleeBase()
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

void UEGA_AIMeleeBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	OnGameplayAbilityEnded.AddLambda([&](UGameplayAbility* GameplayAbility)
	{
		if (IsValid(MontageTask)) MontageTask->EndTask();
		if (IsValid(GameplayEventTask)) GameplayEventTask->EndTask();

		if (AAIController* Controller = Enemy->GetController<AAIController>()) {
			Controller->SetFocus(const_cast<ABaseCharacter*>(Enemy->GetTarget()));
		}
	});

	CommitAbility(Handle, ActorInfo, ActivationInfo);

	Enemy = CastChecked<ABaseNPC>(CurrentActorInfo->OwnerActor);
	Enemy->SetMeleeSphereRadius(SphereInfo.SphereRadius);
	Enemy->SetMeleeSphereOffset(SphereInfo.SphereOffset);

	if (AAIController* Controller = Enemy->GetController<AAIController>())
		Controller->ClearFocus(2);

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage, MontagePlayingRate);
	MontageTask->OnCompleted.AddDynamic(this, &UEGA_AIMeleeBase::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UEGA_AIMeleeBase::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UEGA_AIMeleeBase::OnMontageCompleted);

	GameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.MeleeStrike")));
	GameplayEventTask->EventReceived.AddDynamic(this, &UEGA_AIMeleeBase::OnGameplayEventReceived);

	MontageTask->ReadyForActivation();
	GameplayEventTask->ReadyForActivation();
}

void UEGA_AIMeleeBase::OnMontageCompleted()
{
	if (HasAuthority(&CurrentActivationInfo))
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UEGA_AIMeleeBase::OnGameplayEventReceived(FGameplayEventData GameplayEventData)
{
	ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, CreateDamageEffect(DamageEffect, DamageInfo), GameplayEventData.TargetData);
}
