#include "ProjetHiver/Characters/Abilities/Players/Bow/EGA_BuffRapidFire.h"
#include "ProjetHiver/Characters/Abilities/Players/EGA_RangeBase.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"

UEGA_BuffRapidFire::UEGA_BuffRapidFire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FGameplayTag BuffRapidFireTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.Bow.RapidFire"));

	AbilityTags.AddTag(BuffRapidFireTag);
	ActivationOwnedTags.AddTag(BuffRapidFireTag);

	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsGrounded")));
}

void UEGA_BuffRapidFire::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	OnGameplayAbilityEnded.AddLambda([&](UGameplayAbility* GameplayAbility)
	{
		if (IsValid(StartBuffMontageTask)) StartBuffMontageTask->EndTask();
		if (IsValid(WaitStartBuffEvent)) WaitStartBuffEvent->EndTask();
		if (IsValid(WaitEndBuffTask)) WaitEndBuffTask->EndTask();

		BP_RemoveGameplayEffectFromOwnerWithHandle(BuffEffectHandle);
		Source->RemoveComboGivenAbilitiesFromBuff();
	});

	Source = Cast<APlayerCharacter>(GetOwningActorFromActorInfo());
	if (!IsValid(Source)) CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);

	InitCooldownOnWidget();

	StartBuffMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, StartBuffMontage, MontagePlayingRate);

	WaitStartBuffEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.BuffRapidFire")));
	WaitStartBuffEvent->EventReceived.AddDynamic(this, &UEGA_BuffRapidFire::OnStartBuffEventReceived);

	StartBuffMontageTask->ReadyForActivation();
	WaitStartBuffEvent->ReadyForActivation();
}

void UEGA_BuffRapidFire::OnStartBuffEventReceived(FGameplayEventData GameplayEventData)
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		//Application de l'effet de boost sur les stats
		if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
			CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);

		BuffEffectHandle = ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,
			CreateBuffEffect(BuffEffect, 1.f, 0.f, MaxSpeedMultiplier, MaxSpeedMultiplierBoostByLevel,
				PhysicalPowerMultiplier, PhysicalPowerMultiplierBoostByLevel, 1.f, 0.f, 1.f, 0.f, 1.f, 0.f));

		//Changement des attaques de bases du joueur par le serv
		Source->SetComboGivenAbilitiesFromBuff(NewRangeBaseAttacks);
		WaitEndBuffTask = UAbilityTask_WaitDelay::WaitDelay(this, Duration);
		WaitEndBuffTask->OnFinish.AddDynamic(this, &UEGA_BuffRapidFire::OnEndBuff);
		WaitEndBuffTask->ReadyForActivation();
	}
}

void UEGA_BuffRapidFire::OnEndBuff()
{
	if (HasAuthority(&CurrentActivationInfo))
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
