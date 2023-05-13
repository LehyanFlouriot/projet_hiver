#pragma once

#include "CoreMinimal.h"
#include "ItemType.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Weapon,
	Earrings,
	Armor,
	Gloves,
	Belt,
	Ring,
	Necklace,
	Trash,
	HealSeed
};
