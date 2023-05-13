#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TrashItemRow.generated.h"

USTRUCT(BlueprintType)
struct PROJETHIVER_API FTrashItemRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Name"))
	FText Name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Description"))
	FText Description;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Icon", MakeStructureDefaultValue="None"))
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Loot Actor", MakeStructureDefaultValue="None"))
	TSubclassOf<class AReplicatedStaticMeshActor> LootActor;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Base Sell Value", MakeStructureDefaultValue="0.000000"))
	int BaseSellValue = 0;
};
