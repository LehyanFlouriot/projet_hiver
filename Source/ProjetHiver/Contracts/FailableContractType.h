#pragma once

#include "CoreMinimal.h"
#include "FailableContractType.generated.h"

UENUM(BlueprintType)
enum class EFailableContractType : uint8
{
	Time,
	Potion,
	Resurrect
};
