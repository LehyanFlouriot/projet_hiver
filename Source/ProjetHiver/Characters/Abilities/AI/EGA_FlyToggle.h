#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "EGA_FlyToggle.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

UCLASS()
class PROJETHIVER_API UEGA_FlyToggle : public UEfhorisGameplayAbility
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	UAnimMontage* LandingMontage;

	UPROPERTY(EditAnywhere)
	UAnimMontage* TakeOffMontage;

	UPROPERTY(EditAnywhere)
	float MontagePlayingRate = 1.f;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UFUNCTION()
	void OnMontageCompleted();

public:
	UEGA_FlyToggle();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
};
