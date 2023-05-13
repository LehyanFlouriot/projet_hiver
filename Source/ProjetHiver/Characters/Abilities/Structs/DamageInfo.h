#pragma once

#include "CoreMinimal.h"
#include "DamageInfo.generated.h"

USTRUCT(BlueprintType)
struct FDamageInfo
{
	GENERATED_BODY()

	FDamageInfo() :
		PhysicalDamage(0.f), PhysicalDamageBonusPerLevel(0.f), PhysicalPowerScaling(0.f),
		MagicalDamage(0.f), MagicalDamageBonusPerLevel(0.f),  MagicalPowerScaling(0.f) {}

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float PhysicalDamage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float PhysicalDamageBonusPerLevel;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float PhysicalPowerScaling;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MagicalDamage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MagicalDamageBonusPerLevel;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MagicalPowerScaling;
};
