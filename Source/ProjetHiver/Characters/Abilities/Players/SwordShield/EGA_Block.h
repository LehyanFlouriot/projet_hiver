#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "EGA_Block.generated.h"

class UAbilityTask_WaitDelay;
class UAbilityTask_WaitGameplayEvent;
class UAbilityTask_WaitInputRelease;

UCLASS()
class PROJETHIVER_API UEGA_Block final : public UEfhorisGameplayAbility
{
	GENERATED_BODY()

public:
	UEGA_Block();

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> BlockingEffect;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> ParryingEffect;

	UPROPERTY(EditAnywhere)
	float ParryingLength;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> StunEffect;

	UPROPERTY(EditAnywhere)
	float StunLength;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitDelay> ParryingDelayTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ParryEventTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitInputRelease> InputReleaseTask;

	UFUNCTION()
	void OnDelayFinish();

	UFUNCTION()
	void OnParryEventReceived(FGameplayEventData EventData);

	UFUNCTION()
	void OnRelease(float TimeHeld);

private:
	UPROPERTY(Transient)
	bool bShouldRemoveParryingEffect;

	UPROPERTY(Transient)
	FActiveGameplayEffectHandle BlockingEffectHandle;

	UPROPERTY(Transient)
	FActiveGameplayEffectHandle ParryingEffectHandle;
};
