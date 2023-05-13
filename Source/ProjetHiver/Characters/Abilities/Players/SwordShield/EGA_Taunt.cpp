#include "EGA_Taunt.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "ProjetHiver/Characters/BaseNPC.h"

UEGA_Taunt::UEGA_Taunt()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FGameplayTag TauntTag = FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.SwordShield.Taunt"));

	AbilityTags.AddTag(TauntTag);
	ActivationOwnedTags.AddTag(TauntTag);

	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action")));

	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsGrounded")));
}

void UEGA_Taunt::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	OnGameplayAbilityEnded.AddLambda([&](UGameplayAbility* GameplayAbility)
	{
		if (IsValid(MontageTask)) MontageTask->EndTask();
		if (IsValid(WaitTauntEvent)) WaitTauntEvent->EndTask();
	});

	if (!CommitCheck(Handle, ActorInfo, ActivationInfo))
		if (HasAuthority(&CurrentActivationInfo))
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);

	InitCooldownOnWidget();

	Source = Cast<ABaseCharacter>(GetOwningActorFromActorInfo());
	if (!IsValid(Source)) CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, TauntMontage, MontagePlayingRate);
	MontageTask->OnCompleted.AddDynamic(this, &UEGA_Taunt::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UEGA_Taunt::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UEGA_Taunt::OnMontageCompleted);

	WaitTauntEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.Taunt")));
	WaitTauntEvent->EventReceived.AddDynamic(this, &UEGA_Taunt::OnGameplayEventReceived);

	MontageTask->ReadyForActivation();
	WaitTauntEvent->ReadyForActivation();
}

void UEGA_Taunt::OnMontageCompleted()
{
	CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);

	if (HasAuthority(&CurrentActivationInfo))
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UEGA_Taunt::OnGameplayEventReceived(FGameplayEventData GameplayEventData)
{
	if (HasAuthority(&CurrentActivationInfo))
		if (const ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwningActorFromActorInfo()))
		{
			const FVector TauntSphere = GetOwningActorFromActorInfo()->GetActorLocation() + SphereInfo.SphereOffset;

			TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
			ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(GetOwningActorFromActorInfo());

			TArray<AActor*> OutActors;

			FColor Color;

			if (UKismetSystemLibrary::SphereOverlapActors(GetWorld(), TauntSphere, SphereInfo.SphereRadius, ObjectTypes, ABaseCharacter::StaticClass(), ActorsToIgnore, OutActors))
			{
				for (AActor* Actor : OutActors)
					if (ABaseNPC* Target = Cast<ABaseNPC>(Actor); IsValid(Target))
						if (CastChecked<ABaseCharacter>(GetAvatarActorFromActorInfo())->GetFactionTag() != Target->GetFactionTag() && Target->IsAliveHealth())
								Target->Taunt(Source, TauntStrength);

				Color = FColor::Green;
			}
			else
				Color = FColor::Red;

			if (Character->GetDebugAbilitiesHitbox())
				DrawDebugSphere(GetWorld(), TauntSphere, SphereInfo.SphereRadius, 10, Color, false, 3.f);
		}
}
