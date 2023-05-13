#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Characters/Abilities/Structs/DamageInfo.h"
#include "ProjetHiver/Characters/Abilities/Structs/SphereInfo.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "EGA_MultiHitMeleeBase.generated.h"

class ABaseCharacter;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

UCLASS()
class PROJETHIVER_API UEGA_MultiHitMeleeBase : public UEfhorisGameplayAbility
{
	GENERATED_BODY()

public:
	UEGA_MultiHitMeleeBase();

	UPROPERTY(EditAnywhere)
	TArray<FDamageInfo> DamageInfos;

	UPROPERTY(EditAnywhere)
	TArray<FSphereInfo> SphereInfos;

	UPROPERTY(EditAnywhere)
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere)
	float MontagePlayingRate = 1.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditAnywhere)
	bool bShouldTP = false;

	UPROPERTY(EditAnywhere, meta=(EditCondition=bShouldTP))
	float TPOffset = 100.f;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

protected:
	UPROPERTY(Transient)
	int AbilityProgress;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> MeleeStrikeEventTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> TPBackEventTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ProgressMultiHitEventTask;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	virtual void OnMeleeStrikeEventReceived(FGameplayEventData GameplayEventData);

	UFUNCTION()
	void OnProgressMultiHitEventReceived(FGameplayEventData GameplayEventData);

	UFUNCTION()
	void OnTPBackEventReceived(FGameplayEventData GameplayEventData);
};
