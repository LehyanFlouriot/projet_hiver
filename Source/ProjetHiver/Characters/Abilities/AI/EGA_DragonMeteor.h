#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "EGA_DragonMeteor.generated.h"

class ARangeBaseProjectile;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UAbilityTask_WaitDelay;

UCLASS()
class PROJETHIVER_API UEGA_DragonMeteor : public UEfhorisGameplayAbility
{
	GENERATED_BODY()

public:
	UEGA_DragonMeteor();

	UPROPERTY(EditAnywhere)
	FDamageInfo DamageInfo;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ARangeBaseProjectile> ProjectileClass;
	
	UPROPERTY(EditAnywhere)
	float SpawnRadius = 3000.f;

	UPROPERTY(EditAnywhere)
	float SpawnZ = 2000.f;

	UPROPERTY(EditAnywhere)
	float Cooldown = 0.1f;

	UPROPERTY(EditAnywhere)
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere)
	float MontagePlayingRate = 1.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageEffect;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> GameplayEventTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitDelay> WaitDelayTask;

	UPROPERTY()
	bool bCanActivate;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnWaitFinished();

	UFUNCTION()
	void OnGameplayEventReceived(FGameplayEventData GameplayEventData);
};


