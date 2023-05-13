#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MerchantItemPool.generated.h"

USTRUCT(BlueprintType)
struct FMerchantItemPool
{
	GENERATED_BODY()

	FMerchantItemPool()
	{
		NumberOfItemsToDraw = -1;
		ItemsLevelDelta = 0;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UDataTable*> PoolTables;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	/* -1 means all items of this pool */
	int NumberOfItemsToDraw;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	/* The level delta between the level of the spawned items from the pool & the merchant level */
	int ItemsLevelDelta;
};
