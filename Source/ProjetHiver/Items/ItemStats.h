#pragma once

#include "CoreMinimal.h"
#include "ItemStats.generated.h"

USTRUCT(BlueprintType)
struct FItemStats
{
	GENERATED_BODY()

	FItemStats();
	FItemStats(const int PhysicalPower, const int PhysicalDefense, const int BlockingPhysicalDefense,
			   const int MagicalPower, const int MagicalDefense, const int BlockingMagicalDefense,
			   const int MaxHealth, const int MaxPotionCharge, const int MovementSpeed);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Physical Power", MakeStructureDefaultValue="0.000000"))
	int PhysicalPower;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Physical Defense", MakeStructureDefaultValue="0.000000"))
	int PhysicalDefense;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Blocking Physical Defense", MakeStructureDefaultValue = "0.000000"))
	int BlockingPhysicalDefense;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Magical Power", MakeStructureDefaultValue = "0.000000"))
	int MagicalPower;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Magical Defense", MakeStructureDefaultValue = "0.000000"))
	int MagicalDefense;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Blocking Magical Defense", MakeStructureDefaultValue = "0.000000"))
	int BlockingMagicalDefense;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Max Health", MakeStructureDefaultValue="0.000000"))
	int MaxHealth;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Max Potion Charge", MakeStructureDefaultValue="0.000000"))
	int MaxPotionCharge;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(DisplayName="Movement Speed", MakeStructureDefaultValue="0.000000"))
	int MovementSpeed;

	friend FItemStats operator+(const FItemStats& ItemStatsA, const FItemStats& ItemStatsB)
	{
		return FItemStats{
			ItemStatsA.PhysicalPower + ItemStatsB.PhysicalPower,
			ItemStatsA.PhysicalDefense + ItemStatsB.PhysicalDefense,
			ItemStatsA.BlockingPhysicalDefense + ItemStatsB.BlockingPhysicalDefense,
			ItemStatsA.MagicalPower + ItemStatsB.MagicalPower,
			ItemStatsA.MagicalDefense + ItemStatsB.MagicalDefense,
			ItemStatsA.BlockingMagicalDefense + ItemStatsB.BlockingMagicalDefense,
			ItemStatsA.MaxHealth + ItemStatsB.MaxHealth,
			ItemStatsA.MaxPotionCharge + ItemStatsB.MaxPotionCharge,
			ItemStatsA.MovementSpeed + ItemStatsB.MovementSpeed };
	}
	friend FItemStats operator/(const FItemStats& ItemStatsA, const int Divide)
	{
		return FItemStats{
			ItemStatsA.PhysicalPower / Divide,
			ItemStatsA.PhysicalDefense / Divide,
			ItemStatsA.BlockingPhysicalDefense / Divide,
			ItemStatsA.MagicalPower / Divide,
			ItemStatsA.MagicalDefense / Divide,
			ItemStatsA.BlockingMagicalDefense / Divide,
			ItemStatsA.MaxHealth / Divide,
			ItemStatsA.MaxPotionCharge / Divide,
			ItemStatsA.MovementSpeed / Divide };
	}
};
