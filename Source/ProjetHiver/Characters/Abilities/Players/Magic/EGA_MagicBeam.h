#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Characters/Abilities/Structs/DamageInfo.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "EGA_MagicBeam.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitDelay;
class UAbilityTask_WaitInputRelease;

UCLASS()
class PROJETHIVER_API UEGA_MagicBeam final : public UEfhorisGameplayAbility
{
	GENERATED_BODY()

public:
	UEGA_MagicBeam();

	UPROPERTY(EditAnywhere)
	float MaxRange = 1000.0f;

	UPROPERTY(EditAnywhere)
	float TickLength = 0.f;

	UPROPERTY(EditAnywhere)
	FDamageInfo DamageInfo;

	UPROPERTY(EditAnywhere)
	UAnimMontage* StartAimingMontage;

	UPROPERTY(EditAnywhere)
	UAnimMontage* AimingLoopMontage;

	UPROPERTY(EditAnywhere)
	float StartAnimationPlayingRate = 1.f;

	UPROPERTY(EditAnywhere)
	float LoopingAnimationPlayingRate = 1.f;

	UPROPERTY(EditAnywhere)
	FGameplayTag BeamCueTag;

	UPROPERTY(EditAnywhere)
	FName SocketToFireFrom = FName("LeftHandSocket");

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> SlowEffect;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(Transient)
	FActiveGameplayEffectHandle SlowEffectHandle;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> StartMontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> LoopMontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitDelay> WaitDelayTask;
	
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitInputRelease> WaitInputReleaseTask;

	UFUNCTION()
	void OnStartMontageBlendOut();

	UFUNCTION()
	void OnDelayFinished();

	UFUNCTION()
	void OnInputRelease(float DeltaTime);

	UFUNCTION()
	void CreateTickTask();
};
