#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "DamageExecutionCalculation.generated.h"

UCLASS()
class PROJETHIVER_API UDamageExecutionCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefensePercentageLimit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinimumDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageRandomPercentage;

public:
	UDamageExecutionCalculation();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
