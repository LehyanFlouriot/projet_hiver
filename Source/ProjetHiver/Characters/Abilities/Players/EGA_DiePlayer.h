#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "EGA_DiePlayer.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitDelay;
class UAbilityTask_WaitGameplayEvent;

UCLASS()
class PROJETHIVER_API UEGA_DiePlayer final : public UEfhorisGameplayAbility
{
	GENERATED_BODY()

public:
	UEGA_DiePlayer();

	UPROPERTY(EditAnywhere)
	UAnimMontage* KnockdownMontage;

	UPROPERTY(EditAnywhere)
	float KnockdownMontagePlayingRate = 1.f;

	UPROPERTY(EditAnywhere)
	UAnimMontage* GetupMontage;

	UPROPERTY(EditAnywhere)
	float GetupMontagePlayingRate = 0.6f;

	UPROPERTY(EditAnywhere)
	FGameplayTag DyingTag;

	UPROPERTY(EditAnywhere)
	FGameplayTag DeadTag;

	UPROPERTY(EditAnywhere)
	FGameplayTag InvincibleTag;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> HealEffect;

	UPROPERTY(EditAnywhere)
	float HealCoefficient;

	UPROPERTY(EditAnywhere)
	float DyingTime;

	UPROPERTY(EditAnywhere)
	float ReviveTime;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(Transient)
	TObjectPtr<class UAbilityTask_PlayMontageAndWait> KnockdownMontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> GetupMontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> StartInteractEventTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> StopInteractEventTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitForReviveEventTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitDelay> DyingDelayTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitDelay> ReviveDelayTask;

	UPROPERTY(Transient)
	bool bIsInteractedWith;

	UPROPERTY(Transient)
	bool bShouldDieOnRelease;

	UFUNCTION()
	void OnKnockdownMontageCompleted();

	UFUNCTION()
	void OnGetupMontageCompleted();

	UFUNCTION()
	void OnDyingDelayFinished();

	UFUNCTION()
	void OnReviveDelayFinished();

	UFUNCTION()
	void OnStartInteractReceived(FGameplayEventData GameplayEventData);

	UFUNCTION()
	void OnStopInteractReceived(FGameplayEventData GameplayEventData);

	UFUNCTION()
	void OnWaitForReviveReceived(FGameplayEventData GameplayEventData);
};
