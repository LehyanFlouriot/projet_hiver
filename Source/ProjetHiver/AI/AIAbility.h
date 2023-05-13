#pragma once

#include "CoreMinimal.h"
#include "AIAbility.generated.h"

class UEfhorisGameplayAbility;

USTRUCT(BlueprintType)
struct PROJETHIVER_API FAIAbility
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UEfhorisGameplayAbility> Combo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range {};
};
