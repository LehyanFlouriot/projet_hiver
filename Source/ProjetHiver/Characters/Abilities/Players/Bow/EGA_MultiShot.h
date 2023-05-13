#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Characters/Abilities/Structs/DamageInfo.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "EGA_MultiShot.generated.h"

class ARangeBumpProjectile;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

UCLASS()
class PROJETHIVER_API UEGA_MultiShot : public UEfhorisGameplayAbility
{
	GENERATED_BODY()

public:
	UEGA_MultiShot();

	UPROPERTY(EditAnywhere)
	float Range;

	UPROPERTY(EditAnywhere)
	FDamageInfo DamageInfo;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ARangeBumpProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere)
	FName SocketToSpawnProjectile;

	UPROPERTY(EditAnywhere)
	int NbProjectiles = 3;

	UPROPERTY(EditAnywhere)
	float SpreadAngleInDegree = 120.f;

	UPROPERTY(EditAnywhere)
	float BumpForce = 1000.f;

	UPROPERTY(EditAnywhere)
	float UpCoefficient = 0.5f;

	UPROPERTY(EditAnywhere)
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere)
	float MontagePlayingRate = 1.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> SlowEffect;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> GameplayEventTask;

	UPROPERTY(Transient)
	FActiveGameplayEffectHandle SlowEffectHandle;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnGameplayEventReceived(FGameplayEventData GameplayEventData);
};
