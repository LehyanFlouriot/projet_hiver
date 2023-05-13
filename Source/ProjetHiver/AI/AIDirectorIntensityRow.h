#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ProjetHiver/MapGeneration/Gizmo.h"
#include "AIDirectorIntensityRow.generated.h"

/**
 * Row for representing the data for each state of intensity.
 */
USTRUCT(BlueprintType)
struct PROJETHIVER_API FAIDirectorIntensityRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<EGizmoType, int> SpawnRates;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int MaxGroupMember {};

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int MinGroupMember {};
};
