#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Characters/Abilities/Players/AimedAOE/EGA_AimedAOEBase.h"
#include "EGA_AimedAOETick.generated.h"

class ABaseCharacter;
class UAbilityTask_Repeat;

UCLASS()
class PROJETHIVER_API UEGA_AimedAOETick : public UEGA_AimedAOEBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	float Duration = 5.f;

	UPROPERTY(EditAnywhere)
	float TickLength = 0.5f;

	UPROPERTY(EditAnywhere)
	FGameplayTag CastGameplayCueTag;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_Repeat> RepeatApplyEffectTask;

	UFUNCTION()
	void OnApplyEffectPerformAction(int32 ActionNumber);

	UFUNCTION()
	void OnApplyEffectFinished(int32 ActionNumber);

	virtual void ExecuteAOE(const FGameplayAbilityTargetDataHandle& Data) override;

	UFUNCTION()
	virtual void ApplyEffectToNotInvincible(const ABaseCharacter* Source, ABaseCharacter* Target);

	UFUNCTION()
	virtual void ApplyEffectToAll(const ABaseCharacter* Source, ABaseCharacter* Target) {}
};
