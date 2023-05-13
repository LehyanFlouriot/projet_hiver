#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Items/ItemInfo.h"
#include "PlayerSave.generated.h"

USTRUCT(BlueprintType)
struct FPlayerSave
{
	GENERATED_BODY()

	/* Id */
	UPROPERTY()
	FString PlayerId{};

	/* Inventory */
	UPROPERTY()
	FItemInfo ArmorKit{};

	UPROPERTY()
	FItemInfo WeaponKit{};

	UPROPERTY()
	FItemInfo Belt {};

	UPROPERTY()
	FItemInfo Gloves {};

	UPROPERTY()
	FItemInfo Ring {};

	UPROPERTY()
	FItemInfo Earrings {};

	UPROPERTY()
	FItemInfo Necklace {};

	/* Position */
	UPROPERTY()
	FTransform Transform{};

	UPROPERTY()
	float HealthPercentage = 1.f;

	UPROPERTY()
	int NbPotionCharges = 3;

	UPROPERTY()
	bool bIsDead{};

	UPROPERTY()
	int CurrentLevel = 1;
};
