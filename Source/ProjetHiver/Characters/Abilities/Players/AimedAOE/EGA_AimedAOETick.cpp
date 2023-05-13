#include "EGA_AimedAOETick.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_Repeat.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProjetHiver/Characters/BaseCharacter.h"

void UEGA_AimedAOETick::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                        const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	OnGameplayAbilityEnded.AddLambda([&](UGameplayAbility* GameplayAbility)
	{
		if (IsValid(RepeatApplyEffectTask)) RepeatApplyEffectTask->EndTask();
	});
}

void UEGA_AimedAOETick::OnApplyEffectPerformAction(int32 ActionNumber)
{
	if (HasAuthority(&CurrentActivationInfo))
		if (const ABaseCharacter* Source = Cast<ABaseCharacter>(GetOwningActorFromActorInfo()))
		{
			TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
			ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(GetOwningActorFromActorInfo());

			TArray<AActor*> OutActors;

			FColor Color;

			if (UKismetSystemLibrary::SphereOverlapActors(GetWorld(), AoELocation, SphereInfo.SphereRadius, ObjectTypes, ABaseCharacter::StaticClass(), ActorsToIgnore, OutActors))
			{
				for (AActor* Actor : OutActors)
					if (ABaseCharacter* Target = Cast<ABaseCharacter>(Actor); IsValid(Target))
						if (Source->GetFactionTag() != Target->GetFactionTag() &&
							Target->IsAliveHealth())
						{
							if (!Target->GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsInvincible"))))
								ApplyEffectToNotInvincible(Source, Target);

							ApplyEffectToAll(Source, Target);
						}

				Color = FColor::Green;
			}
			else
				Color = FColor::Red;

			if (Source->GetDebugAbilitiesHitbox())
				DrawDebugSphere(GetWorld(), AoELocation, SphereInfo.SphereRadius, 10, Color, false, 3.f);
		}
}

void UEGA_AimedAOETick::OnApplyEffectFinished(int32 ActionNumber)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UEGA_AimedAOETick::ExecuteAOE(const FGameplayAbilityTargetDataHandle& Data)
{
	Super::ExecuteAOE(Data);

	FGameplayCueParameters GameplayCueParameters;
	GameplayCueParameters.Location = AoELocation;
	K2_ExecuteGameplayCueWithParams(CastGameplayCueTag, GameplayCueParameters);

	if (HasAuthority(&CurrentActivationInfo))
	{
		RepeatApplyEffectTask = UAbilityTask_Repeat::RepeatAction(this, TickLength, Duration / TickLength);
		RepeatApplyEffectTask->OnPerformAction.AddDynamic(this, &UEGA_AimedAOETick::OnApplyEffectPerformAction);
		RepeatApplyEffectTask->OnFinished.AddDynamic(this, &UEGA_AimedAOETick::OnApplyEffectFinished);

		RepeatApplyEffectTask->ReadyForActivation();
	}
}

void UEGA_AimedAOETick::ApplyEffectToNotInvincible(const ABaseCharacter* Source, ABaseCharacter* Target)
{
	ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, CreateDamageEffect(DamageEffect, DamageInfo), UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(Target));
}
