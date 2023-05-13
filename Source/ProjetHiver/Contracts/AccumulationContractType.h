#pragma once

#include "CoreMinimal.h"
#include "AccumulationContractType.generated.h"

UENUM(BlueprintType)
enum class EAccumulationContractType : uint8
{
	Kill,
	Chest,
	Tile
};
