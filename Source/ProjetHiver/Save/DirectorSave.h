#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/MapGeneration/Gizmo.h"
#include "DirectorSave.generated.h"

class AEnemy;
class AGroup;

USTRUCT()
struct PROJETHIVER_API FChestSave
{
	GENERATED_BODY()

	UPROPERTY()
	bool bOpened = false;

	UPROPERTY()
	FString GizmoName{};

	UPROPERTY()
	FVector Tile = FVector(0);
};

USTRUCT()
struct PROJETHIVER_API FEnemySave
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform Transform{};

	UPROPERTY()
	FString GizmoName {};

	UPROPERTY()
	FVector Tile = FVector(0);

	UPROPERTY()
	FTopLevelAssetPath EnemyClassPath {};

	UPROPERTY()
	float HealthPercentage = 1.f;

	UPROPERTY()
	FString TargetId {};
};

USTRUCT()
struct PROJETHIVER_API FGroupSave
{
	GENERATED_BODY()

	UPROPERTY()
	FString GizmoName {};

	UPROPERTY()
	FVector Tile = FVector(0);

	UPROPERTY()
	FTopLevelAssetPath GroupClassPath {};

	UPROPERTY()
	TArray<FEnemySave> Wolfs{};
};

USTRUCT()
struct PROJETHIVER_API FAloneEnemySave
{
	GENERATED_BODY()

	UPROPERTY()
	bool bIsGroup = false;

	UPROPERTY()
	FGroupSave Group {};

	UPROPERTY()
	FEnemySave Enemy {};
};

USTRUCT()
struct PROJETHIVER_API FBossSave
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform Transform {};

	UPROPERTY()
	FString GizmoName {};

	UPROPERTY()
	FVector Tile = FVector(0);

	UPROPERTY()
	float HealthPercentage = 1.f;

	UPROPERTY()
	int Phase{};
};

USTRUCT()
struct PROJETHIVER_API FKevinSave
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform Transform {};

	UPROPERTY()
	FString GizmoName {};

	UPROPERTY()
	FVector Tile = FVector(0);

	UPROPERTY()
	FString TargetId {};
};

USTRUCT()
struct PROJETHIVER_API FMercenarySave
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform Transform {};

	UPROPERTY()
	FString GizmoName {};

	UPROPERTY()
	FVector Tile = FVector(0);

	UPROPERTY()
	float HealthPercentage = 1.f;

	UPROPERTY()
	float NbPotionCharges{};

	UPROPERTY()
	FString MasterId {};
};

USTRUCT()
struct PROJETHIVER_API FGizmoSave
{
	GENERATED_BODY()

	UPROPERTY()
	EGizmoType Type {};

	UPROPERTY()
	FString GizmoName {};

	UPROPERTY()
	FVector Tile = FVector(0);
};

USTRUCT()
struct PROJETHIVER_API FGizmosSave
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FChestSave> Chests{};

	UPROPERTY()
	TArray<FAloneEnemySave> Enemies{};

	UPROPERTY()
	TArray<FEnemySave> SubBosses{};

	UPROPERTY()
	TArray<FBossSave> Bosses{};

	UPROPERTY()
	TArray<FKevinSave> Kevins{};

	UPROPERTY()
	TArray<FMercenarySave> Mercenaries{};

	UPROPERTY()
	TArray<FGizmoSave> OtherGizmos{};

	void Append(const FGizmosSave& Other) {
		Chests.Append(Other.Chests);
		Enemies.Append(Other.Enemies);
		SubBosses.Append(Other.SubBosses);
		Bosses.Append(Other.Bosses);
		Mercenaries.Append(Other.Mercenaries);
		OtherGizmos.Append(Other.OtherGizmos);
	}
};

/**
 * 
 */
USTRUCT()
struct PROJETHIVER_API FDirectorSave
{
	GENERATED_BODY()

	UPROPERTY()
	int IntensityState = 0;

	UPROPERTY()
	int CurrentNumberOfRoomVisited{};

	UPROPERTY()
	float CurrentDifficulty{};

	UPROPERTY()
	TArray<FVector> VisitedTiles{};

	UPROPERTY()
	TArray<FVector> LoadedTiles{};

	UPROPERTY()
	FGizmosSave GizmosSave {};

	bool IsEmpty() const noexcept {
		return LoadedTiles.IsEmpty();
	}
};
