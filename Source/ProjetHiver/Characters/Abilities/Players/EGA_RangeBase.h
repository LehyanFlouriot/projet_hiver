#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Characters/Abilities/Structs/DamageInfo.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "EGA_RangeBase.generated.h"

class ARangeBaseProjectile;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

UCLASS()
class PROJETHIVER_API UEGA_RangeBase : public UEfhorisGameplayAbility
{
	GENERATED_BODY()

public:
	UEGA_RangeBase();

	UPROPERTY(EditAnywhere)
	float Range;

	UPROPERTY(EditAnywhere)
	FDamageInfo DamageInfo;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ARangeBaseProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere)
	FName SocketToSpawnProjectile;

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
	virtual void OnGameplayEventReceived(FGameplayEventData GameplayEventData);
};
