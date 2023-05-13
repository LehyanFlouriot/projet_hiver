#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Characters/Abilities/Structs/DamageInfo.h"
#include "ProjetHiver/Characters/Abilities/Structs/SphereInfo.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "EGA_AimedAOEBase.generated.h"

class AGameplayAbilityTargetActor_GroundTrace;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_Repeat;
class UAbilityTask_WaitDelay;
class UAbilityTask_WaitTargetData;
struct FActiveGameplayEffectHandle;
struct FGameplayAbilityTargetDataHandle;
class FWaitTargetDataDelegate;

UCLASS()
class PROJETHIVER_API UEGA_AimedAOEBase : public UEfhorisGameplayAbility
{
	GENERATED_BODY()

public:
	UEGA_AimedAOEBase();

	UPROPERTY(EditAnywhere)
	float MaxRange = 5000.f;

	UPROPERTY(EditAnywhere)
	float AimingOffset = 200.f;

	UPROPERTY(EditAnywhere)
	FDamageInfo DamageInfo;

	UPROPERTY(EditAnywhere)
	FSphereInfo SphereInfo;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AGameplayAbilityTargetActor_GroundTrace> TargetActorClass;

	UPROPERTY(EditAnywhere)
	UAnimMontage* StartAimingMontage;

	UPROPERTY(EditAnywhere)
	UAnimMontage* AimingLoopMontage;

	UPROPERTY(EditAnywhere)
	UAnimMontage* EndAimingMontage;

	UPROPERTY(EditAnywhere)
	float StartAnimationPlayingRate = 1.f;

	UPROPERTY(EditAnywhere)
	float LoopingAnimationPlayingRate = 1.f;

	UPROPERTY(EditAnywhere)
	float EndAnimationPlayingRate = 1.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> IsAimingEffect;

	UPROPERTY(EditAnywhere)
	float DelayAfterConfirmOrCancel = 0.2f;

	UPROPERTY(EditAnywhere)
	int CooldownSlot = -1;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(Transient)
	FActiveGameplayEffectHandle IsAimingEffectHandle;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> StartMontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> LoopMontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> EndMontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitDelay> WaitDelayBeforeIsAimingRemovalTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitTargetData> WaitTargetDataEventTask;

	UPROPERTY(Transient)
	bool bShouldEndAbility;

	UPROPERTY(Transient)
	FVector AoELocation;

	UFUNCTION()
	void OnStartMontageBlendOut();

	UFUNCTION()
	void OnValidDataReceived(const FGameplayAbilityTargetDataHandle& Data);

	UFUNCTION()
	void OnCancelledReceived(const FGameplayAbilityTargetDataHandle& Data);

	UFUNCTION()
	void OnWaitDelayBeforeIsAimingRemovalTask();

	UFUNCTION()
	virtual void ExecuteAOE(const FGameplayAbilityTargetDataHandle& Data) {}
};
