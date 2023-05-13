#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Contracts/Contract.h"
#include "ProjetHiver/Contracts/ContractStatus.h"
#include "ProjetHiver/Items/ItemInfo.h"
#include "DirectorSave.h"
#include "GameSave.generated.h"

USTRUCT(BlueprintType)
struct FGameSave
{
	GENERATED_BODY()

	/* Ids */
	UPROPERTY(BlueprintReadOnly)
	FGuid GameId{};

	UPROPERTY(BlueprintReadOnly)
	FString GameName{};

	UPROPERTY(BlueprintReadOnly)
	FString GameCreatorId{};

	UPROPERTY(BlueprintReadOnly)
	FDateTime Date{};

	/* Inventory */
	UPROPERTY(BlueprintReadOnly)
	TArray<FItemInfo> SharedInventory{};

	UPROPERTY(BlueprintReadOnly)
	int Money{};

	UPROPERTY(BlueprintReadOnly)
	FTransform InventoryTransform = FTransform{ FVector{0, 0, 20} };

	UPROPERTY(BlueprintReadOnly)
	FString TargetId;

	/* Map generation */
	UPROPERTY(BlueprintReadOnly)
	int Seed{};

	UPROPERTY(BlueprintReadOnly)
	int CurrentLevel{};

	/* AI Director */
	UPROPERTY(BlueprintReadOnly)
	float DamageTaken{};

	UPROPERTY(BlueprintReadOnly)
	int NbVisitedTiles{};

	/* Contracts */
	UPROPERTY(BlueprintReadOnly)
	TArray<FTopLevelAssetPath> ContractClassPaths{};

	UPROPERTY(BlueprintReadOnly)
	TArray<EContractStatus> ContractStatuses{};

	UPROPERTY(BlueprintReadOnly)
	TArray<int> ContractCurrentValues{};

	/* Doctor */
	UPROPERTY(BlueprintReadOnly)
	int HealAllPlayersCost{};

	bool operator==(const FGameSave& Other) const {
		return GameId.ToString() == Other.GameId.ToString() && GameCreatorId == Other.GameCreatorId;
	}

	bool operator<(const FGameSave & Other) const {
		return Other.Date < Date;
	}
};
