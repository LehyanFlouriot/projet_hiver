#include "EGA_DieAI.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjetHiver/Characters/BaseCharacter.h"
#include "ProjetHiver/GAS/EfhorisAbilitySystemComponent.h"

UEGA_DieAI::UEGA_DieAI()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FGameplayTag DeathTag = FGameplayTag::RequestGameplayTag(FName("Character.Die"));

	AbilityTags.AddTag(DeathTag);
	ActivationOwnedTags.AddTag(DeathTag);

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsDying")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsDead")));

	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action")));

	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action")));
}

void UEGA_DieAI::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	OnGameplayAbilityEnded.AddLambda([&](UGameplayAbility* GameplayAbility)
	{
		if (IsValid(MontageTask)) MontageTask->EndTask();
		CastChecked<ABaseCharacter>(CurrentActorInfo->OwnerActor)->GetCharacterMovement()->DisableMovement();
		ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, MakeOutgoingGameplayEffectSpec(DieEffect));
	});

	CommitAbility(Handle, ActorInfo, ActivationInfo);

	const ABaseCharacter* Source = CastChecked<ABaseCharacter>(ActorInfo->OwnerActor);

	Source->GetAbilitySystemComponent()->CancelAllAbilities(this);
	if (const auto Controller = Source->GetController(); IsValid(Controller)) 
		Controller->StopMovement();
	SetUserToFlyingMode();

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, DieMontage, MontagePlayingRate);
	MontageTask->OnBlendOut.AddDynamic(this, &UEGA_DieAI::OnMontageCompleted);
	MontageTask->OnCompleted.AddDynamic(this, &UEGA_DieAI::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UEGA_DieAI::OnMontageCompleted);
	MontageTask->ReadyForActivation();
}

void UEGA_DieAI::OnMontageCompleted()
{
	if (HasAuthority(&CurrentActivationInfo))
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
