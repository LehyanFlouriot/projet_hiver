#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "EGA_BuffRapidFire.generated.h"

class APlayerCharacter;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitDelay;
class UAbilityTask_WaitGameplayEvent;
class UEGA_RangeBase;

UCLASS()
class PROJETHIVER_API UEGA_BuffRapidFire : public UEfhorisGameplayAbility
{
	GENERATED_BODY()

public:
	UEGA_BuffRapidFire();

protected:
	UPROPERTY(EditAnywhere)
	float Duration;

	UPROPERTY(EditAnywhere)
	UAnimMontage* StartBuffMontage;

	UPROPERTY(EditAnywhere)
	float MontagePlayingRate = 1.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> BuffEffect;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UEfhorisGameplayAbility>> NewRangeBaseAttacks;

	UPROPERTY(EditAnywhere)
	float MaxSpeedMultiplier;

	UPROPERTY(EditAnywhere)
	float MaxSpeedMultiplierBoostByLevel;

	UPROPERTY(EditAnywhere)
	float PhysicalPowerMultiplier;

	UPROPERTY(EditAnywhere)
	float PhysicalPowerMultiplierBoostByLevel;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> StartBuffMontageTask;

	UPROPERTY(Transient)
	FActiveGameplayEffectHandle BuffEffectHandle;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitStartBuffEvent;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitDelay> WaitEndBuffTask;

	UPROPERTY(Transient)
	TObjectPtr<APlayerCharacter> Source;

	UFUNCTION()
	void OnStartBuffEventReceived(FGameplayEventData GameplayEventData);

	UFUNCTION()
	void OnEndBuff();
};
