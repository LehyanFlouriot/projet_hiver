#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Characters/Abilities/Structs/DamageInfo.h"
#include "ProjetHiver/Characters/Abilities/Structs/SphereInfo.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "EGA_AIMeleeBase.generated.h"

class ABaseNPC;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

UCLASS()
class PROJETHIVER_API UEGA_AIMeleeBase final : public UEfhorisGameplayAbility
{
	GENERATED_BODY()
public:
	UEGA_AIMeleeBase();

	UPROPERTY(EditAnywhere, NoClear)
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere)
	float MontagePlayingRate = 1.f;

	UPROPERTY(EditAnywhere, NoClear)
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditAnywhere)
	FDamageInfo DamageInfo;

	UPROPERTY(EditAnywhere)
	FSphereInfo SphereInfo;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> GameplayEventTask;

	UPROPERTY(Transient)
	TWeakObjectPtr<ABaseNPC> Enemy;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnGameplayEventReceived(FGameplayEventData GameplayEventData);
};
