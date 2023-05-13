#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/MapGeneration/Gizmo.h"
#include "ProjetHiver/Random/WeightedActor.h"
#include "ActorsByGizmo.generated.h"

/**
 * Eeach Blueprint Spawnable Classes for each gizmos.
 */
USTRUCT(BlueprintType)
struct PROJETHIVER_API FActorsByGizmo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGizmoType Type {};

	/** Array Containing each actors spawnable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<AWeightedActor>> Actors;

	FActorsByGizmo() = default;

	FActorsByGizmo(EGizmoType T) {
		Type = T;
	}

	bool operator==(const FActorsByGizmo& Other) const {
		return Type == Other.Type;
	}
};
