#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Characters/Abilities/Players/EGA_MultiHitMeleeBase.h"
#include "EGA_MultiHitMeleeWithDodge.generated.h"

UCLASS()
class PROJETHIVER_API UEGA_MultiHitMeleeWithDodge final : public UEGA_MultiHitMeleeBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> InvincibleEffect;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(Transient)
	FActiveGameplayEffectHandle InvincibleEffectHandle;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> InvincibilityBeginEventTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> InvincibilityEndEventTask;

	UFUNCTION()
	void OnInvincibilityBeginEventReceived(FGameplayEventData EventData);

	UFUNCTION()
	void OnInvincibilityEndEventReceived(FGameplayEventData EventData);
};
