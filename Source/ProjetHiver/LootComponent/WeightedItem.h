#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Items/ItemType.h"
#include "WeightedItem.generated.h"

USTRUCT()
struct FWeightedItem
{
	GENERATED_BODY()

	FWeightedItem() : Type(EItemType::Trash), Name(FName{}), Weight(1.f) {}

	UPROPERTY(EditAnywhere)
	EItemType Type;

	UPROPERTY(EditAnywhere)
	FName Name;

	UPROPERTY(EditAnywhere)
	float Weight;
};