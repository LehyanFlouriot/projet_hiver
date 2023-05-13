#pragma once

#include "Engine/DataTable.h"
#include "ProjetHiver/Contracts/ExplorationContractType.h"
#include "Basics.generated.h"

class ABaseBoss;
class ABaseNPC;
class ATile;
class ATileLimit;

UENUM(BlueprintType)
enum class ETileType : uint8
{
	GenericTile UMETA(DisplayName = "GenericTile"),
	StartTile UMETA(DisplayName = "StartTile"),
	EndTile UMETA(DisplayName = "EndTile"),
	SpecialTile UMETA(DisplayName = "SpecialTile"),
	ExplorationContractTile UMETA(DisplayName = "ExplorationContractTile"),
	Num UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EConnection : uint8
{
	N UMETA(DisplayName = "N"),
	S UMETA(DisplayName = "S"),
	W UMETA(DisplayName = "W"),
	E UMETA(DisplayName = "E"),
	Random UMETA(DisplayName = "Random", Hidden),
	Num UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ETileDirection : uint8
{
	N	UMETA(DisplayName = "N"),
	S	UMETA(DisplayName = "S"),
	W	UMETA(DisplayName = "W"),
	E	UMETA(DisplayName = "E"),
	SE	 UMETA(DisplayName = "SE"),
	NE	 UMETA(DisplayName = "NE"),
	NW	 UMETA(DisplayName = "NW"),
	SW	 UMETA(DisplayName = "SW"),
	NS	 UMETA(DisplayName = "NS"),
	EW	 UMETA(DisplayName = "EW"),
	NSEW UMETA(DisplayName = "NSEW"),
	NEW UMETA(DisplayName = "NEW"),
	NSW UMETA(DisplayName = "NSW"),
	SEW UMETA(DisplayName = "SEW"),
	NSE UMETA(DisplayName = "NSE"),
	Num                     UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct PROJETHIVER_API FExplorationContractTileSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta = (ArraySizeEnum = "ETileDirection", EditCondition = "bFixedPreferences", EditConditionHides))
	TSubclassOf<ATile> Tiles[ETileDirection::Num];
};

/** Contains information on the level for levelGenerator*/
USTRUCT(BlueprintType)
struct PROJETHIVER_API FLevelInformation : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Seed", MakeStructureDefaultValue = "0"))
	int32 Seed {};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "CurrentLevel", MakeStructureDefaultValue = "0"))
	int32 CurrentLevel {};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "TileSize", MakeStructureDefaultValue = "0.000000,0.000000,0.000000"))
	FVector TileSize = FVector(0);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "TileAmount", MakeStructureDefaultValue = "0"))
	int32 TileAmount {};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "StartTile", MakeStructureDefaultValue = "None"))
	TSubclassOf<ATile> StartTile;

	UPROPERTY(EditAnywhere, meta = (ArraySizeEnum = "ETileDirection", EditCondition = "bFixedPreferences", EditConditionHides))
	TSubclassOf<ATile> GenericTiles[ETileDirection::Num];

	UPROPERTY(EditAnywhere, meta = (ArraySizeEnum = "ETileDirection", EditCondition = "bFixedPreferences", EditConditionHides))
	TSubclassOf<ATile> SpecialTiles[ETileDirection::Num];

	UPROPERTY(EditAnywhere, meta = (ArraySizeEnum = "EExplorationContractType", EditCondition = "bFixedPreferences", EditConditionHides))
	FExplorationContractTileSet ExplorationContractTileSets[EExplorationContractType::Num];

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "SpecialTilesAmount"))
	int32 SpecialTilesAmount {};

	UPROPERTY(EditAnywhere, meta = (ArraySizeEnum = "ETileDirection", EditCondition = "bFixedPreferences", EditConditionHides))
	TSubclassOf<ATile> EndTiles[ETileDirection::Num];

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "StepBackChance", MakeStructureDefaultValue = "0.000000"))
	int32 StepBackChance {};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "ChanceForExtraConnections", MakeStructureDefaultValue = "0.000000"))
	int32 ChanceForExtraConnections {};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "MaxExtraConnections", MakeStructureDefaultValue = "0"))
	int32 MaxExtraConnections {};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "TileLimitClass"))
	TSubclassOf<ATileLimit> TileLimitClass;
};
