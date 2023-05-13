#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "EGA_Jump.generated.h"

UCLASS()
class PROJETHIVER_API UEGA_Jump final : public UEfhorisGameplayAbility
{
	GENERATED_BODY()

public:
	UEGA_Jump();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
