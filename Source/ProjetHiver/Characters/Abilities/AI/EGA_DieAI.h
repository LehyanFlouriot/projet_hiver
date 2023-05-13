#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "EGA_DieAI.generated.h"

class UAbilityTask_PlayMontageAndWait;

UCLASS()
class PROJETHIVER_API UEGA_DieAI final : public UEfhorisGameplayAbility
{
	GENERATED_BODY()

public:
	UEGA_DieAI();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* DieMontage;

	UPROPERTY(EditAnywhere)
	float MontagePlayingRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DieEffect;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UFUNCTION()
	void OnMontageCompleted();
};
