#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "EGA_AIPotion.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class AMercenary;
class AReplicatedStaticMeshActor;

UCLASS()
class PROJETHIVER_API UEGA_AIPotion : public UEfhorisGameplayAbility
{
	GENERATED_BODY()

public:
	UEGA_AIPotion();

	UPROPERTY(EditAnywhere)
	UAnimMontage* DrinkingMontage;

	UPROPERTY(EditAnywhere)
	float MontagePlayingRate = 1.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> HealEffect;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AReplicatedStaticMeshActor> PotionBottle;

	UPROPERTY(EditAnywhere)
	float HealCoefficient;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> GameplayEventTask;

	UPROPERTY(Transient)
	TObjectPtr<AReplicatedStaticMeshActor> PotionBottleActor;

	UPROPERTY(Transient)
	TWeakObjectPtr<AMercenary> Character;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnGameplayEventReceived(FGameplayEventData GameplayEventData);
};