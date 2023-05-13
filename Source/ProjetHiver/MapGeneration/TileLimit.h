#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TileLimit.generated.h"

UCLASS()
class PROJETHIVER_API ATileLimit : public AActor
{
	GENERATED_BODY()

	ATileLimit() = default;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetDirection(FVector2D FromTile, FVector2D ToTile);
};
