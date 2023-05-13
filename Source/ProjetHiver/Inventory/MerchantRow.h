#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MerchantRow.generated.h"

struct FMerchantItemPool;

USTRUCT(BlueprintType)
struct PROJETHIVER_API FMerchantRow : public FTableRowBase
{
	GENERATED_BODY()

	FMerchantRow() : ItemPools({}), bIsInfinite(false), bGenerateRandomStats(false) {}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Item Pools"))
	TArray<FMerchantItemPool> ItemPools;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Is Infinite"))
	bool bIsInfinite;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Generate Random Stats"))
	bool bGenerateRandomStats;
};
