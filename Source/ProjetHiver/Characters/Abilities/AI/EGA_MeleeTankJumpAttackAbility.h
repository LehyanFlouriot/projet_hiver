#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Characters/Abilities/Players/EGA_MultiHitMeleeBase.h"
#include "EGA_MeleeTankJumpAttackAbility.generated.h"

UCLASS()
class PROJETHIVER_API UEGA_MeleeTankJumpAttackAbility : public UEGA_MultiHitMeleeBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	float ImpulseForce = 200.f;

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

};
