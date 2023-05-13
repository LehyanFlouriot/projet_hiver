#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "EGA_FollowMeteor.generated.h"

class ARangeBaseProjectile;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

UCLASS()
class PROJETHIVER_API UEGA_FollowMeteor : public UEfhorisGameplayAbility
{
	GENERATED_BODY()

public:
	UEGA_FollowMeteor();

	UPROPERTY(EditAnywhere)
	FDamageInfo DamageInfo;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ARangeBaseProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere)
	float Radius = 3000.f;

	UPROPERTY(EditAnywhere)
	float SpawnZ = 2000.f;

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

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnGameplayEventReceived(FGameplayEventData GameplayEventData);
};


