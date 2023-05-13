#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "ProjetHiver/Characters/Abilities/Structs/DamageInfo.h"
#include "EGA_SpreadFire.generated.h"

class ARangeBaseProjectile;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

/*
 * Ability which spawn projectile each frame during anim notify state ANS_SpawnProjectiles
 */
UCLASS()
class PROJETHIVER_API UEGA_SpreadFire : public UEfhorisGameplayAbility
{
	GENERATED_BODY()
public:
	UEGA_SpreadFire();

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

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> SpawnProjectilesEventTask;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnSpawnProjectilesEventReceived(FGameplayEventData GameplayEventData);
};
