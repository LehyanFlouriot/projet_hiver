#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "EGA_Dodge.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

UCLASS()
class PROJETHIVER_API UEGA_Dodge final : public UEfhorisGameplayAbility
{
	GENERATED_BODY()

public:
	UEGA_Dodge();

	UPROPERTY(EditAnywhere)
	UAnimMontage* DodgeMontageF;

	UPROPERTY(EditAnywhere)
	UAnimMontage* DodgeMontageB;

	UPROPERTY(EditAnywhere)
	UAnimMontage* DodgeMontageL;

	UPROPERTY(EditAnywhere)
	UAnimMontage* DodgeMontageR;

	UPROPERTY(EditAnywhere)
	float MontagePlayingRate = 1.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> InvincibleEffect;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(Transient)
	FActiveGameplayEffectHandle InvincibleEffectHandle;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> InvincibilityBeginEventTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> InvincibilityEndEventTask;

	UFUNCTION()
	void OnCompleted();

	UFUNCTION()
	void OnInvincibilityBeginEventReceived(FGameplayEventData EventData);

	UFUNCTION()
	void OnInvincibilityEndEventReceived(FGameplayEventData EventData);
};
