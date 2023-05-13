#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "EGA_HealPotion.generated.h"

class UAbilityTask_NetworkSyncPoint;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class APlayerCharacter;
class AReplicatedStaticMeshActor;

UCLASS()
class PROJETHIVER_API UEGA_HealPotion final : public UEfhorisGameplayAbility
{
	GENERATED_BODY()

public:
	UEGA_HealPotion();

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
	TObjectPtr<UAbilityTask_NetworkSyncPoint> SyncTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> GameplayEventTask;

	UPROPERTY(Transient)
	TObjectPtr<AReplicatedStaticMeshActor> PotionBottleActor;

	UPROPERTY(Transient)
	TWeakObjectPtr<APlayerCharacter> Character;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

	UFUNCTION()
	void OnSync();

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnGameplayEventReceived(FGameplayEventData GameplayEventData);
};