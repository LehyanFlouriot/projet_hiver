#include "ProjetHiver/Characters/Abilities/AI/EGA_FlyToggle.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjetHiver/Characters/DragonBoss.h"

void UEGA_FlyToggle::OnMontageCompleted()
{
	if (ADragonBoss* DragonBoss = Cast<ADragonBoss>(GetOwningActorFromActorInfo()); ensure(IsValid(DragonBoss)))
	{
		if (HasAuthority(&CurrentActivationInfo))
		{
			DragonBoss->SetIsFlying(!DragonBoss->GetIsFlying());
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		}
	}
}

UEGA_FlyToggle::UEGA_FlyToggle()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.DragonBoss.FlyToggle"));

	AbilityTags.AddTag(AttackTag);

	ActivationOwnedTags.AddTag(AttackTag);
}

void UEGA_FlyToggle::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	OnGameplayAbilityEnded.AddLambda([&](UGameplayAbility* GameplayAbility)
		{
			if (IsValid(MontageTask)) MontageTask->EndTask();
		});

	CommitAbility(Handle, ActorInfo, ActivationInfo);

	if(const ADragonBoss* DragonBoss = Cast<ADragonBoss>(GetOwningActorFromActorInfo()); ensure(IsValid(DragonBoss)))
	{
		MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, DragonBoss->GetIsFlying() ? LandingMontage : TakeOffMontage, MontagePlayingRate);
		MontageTask->OnCompleted.AddDynamic(this, &UEGA_FlyToggle::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UEGA_FlyToggle::OnMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &UEGA_FlyToggle::OnMontageCompleted);
	}

	MontageTask->ReadyForActivation();
}
