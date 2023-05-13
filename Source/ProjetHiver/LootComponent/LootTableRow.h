#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ProjetHiver/LootComponent/WeightedItem.h"
#include "LootTableRow.generated.h"

USTRUCT(BlueprintType)
struct PROJETHIVER_API FLootTableRow : public FTableRowBase
{
	GENERATED_BODY()

	FLootTableRow() : Name(FName{}), WeightedItems({}) {}

	UPROPERTY(EditAnywhere, meta=(DisplayName="Name"))
	FName Name;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Weighted Items"))
	TArray<FWeightedItem> WeightedItems;
};
