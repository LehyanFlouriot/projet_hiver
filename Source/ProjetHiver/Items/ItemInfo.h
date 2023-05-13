#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Items/ItemStats.h"
#include "ProjetHiver/Items/ItemType.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "ItemInfo.generated.h"

USTRUCT(BlueprintType)
struct FItemInfo 
{
	GENERATED_BODY()

	inline static int NumFItemInfo = 0;

	FItemInfo()
	{
		Id = ++NumFItemInfo;
		ItemType = EItemType::Trash;
		SellValue = BuyValue = 0.f;
		Stack = 1;
		Level = 0;
		ItemStats = {};
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RowName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Level;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int BuyValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int SellValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Stack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FItemStats ItemStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UEfhorisGameplayAbility> Ability1;

	UPROPERTY(BlueprintReadOnly)
	FTopLevelAssetPath Ability1ClassPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UEfhorisGameplayAbility> Ability2;

	UPROPERTY(BlueprintReadOnly)
	FTopLevelAssetPath Ability2ClassPath;

	bool operator==(const FItemInfo& Other) const noexcept
	{
		return Other.Id == Id;
	}

	static FItemInfo Copy(const FItemInfo& ItemInfo)
	{
		FItemInfo Copy{};

		Copy.RowName = ItemInfo.RowName;
		Copy.ItemType = ItemInfo.ItemType;
		Copy.Level = ItemInfo.Level;
		Copy.BuyValue = ItemInfo.BuyValue;
		Copy.SellValue = ItemInfo.SellValue;
		Copy.Stack = ItemInfo.Stack;
		Copy.ItemStats = ItemInfo.ItemStats;
		Copy.Ability1 = ItemInfo.Ability1;
		Copy.Ability1ClassPath = ItemInfo.Ability1ClassPath;
		Copy.Ability2 = ItemInfo.Ability2;
		Copy.Ability2ClassPath = ItemInfo.Ability2ClassPath;

		return Copy;
	}
};
