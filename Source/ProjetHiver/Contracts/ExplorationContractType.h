#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EExplorationContractType : uint8
{
    MiniBoss                UMETA(DisplayName = "MiniBoss"),
    FetchItem               UMETA(DisplayName = "FetchItem"),
    FetchNPC                UMETA(DisplayName = "FetchNPC"),
    Num                     UMETA(Hidden)
};

