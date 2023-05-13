#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.generated.h"

class UGizmo;
enum class ETileDirection : uint8;
enum class EConnection : uint8;
enum class ETileType : uint8;

/** Contains information about a tile */
USTRUCT(BlueprintType)
struct FTileInformation
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Type", MakeStructureDefaultValue = "GenericTile"))
	ETileType Type {};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Connections", MakeStructureDefaultValue = "((N, False),(S, False),(W, False),(E, False))"))
	TMap<EConnection, bool> Connections;

	/** Represent if the tile was already visited by atleast one player */
	UPROPERTY(BlueprintReadOnly)
	bool bIsVisited = false;

	/** Represent if the mobs are already loaded in this tile */
	UPROPERTY(BlueprintReadOnly)
	bool bIsLoaded = false;

	/** Tile associated */
	UPROPERTY(Transient, BlueprintReadOnly)
	ATile* Tile = nullptr;
};

/** Contains information on tiles to spawn in order to replicate the tiles to clients so they can build them */
USTRUCT(BlueprintType)
struct FTileSpawnInformation
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "TileToSpawn"))
	TSubclassOf<ATile> TileToSpawn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Transform", MakeStructureDefaultValue = "0.000000,0.000000,0.000000|0.000000,0.000000,-0.000000|1.000000,1.000000,1.000000"))
	FTransform Transform;
};
/* Information for the minimap widget */
USTRUCT()
struct FLocatedTile
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FVector2D Position = FVector2D(0,0);

	UPROPERTY(Transient)
	UTexture2D* Texture = nullptr;
};

UCLASS()
class PROJETHIVER_API ATile : public AActor
{
	GENERATED_BODY()

public:	
	ATile();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Metrics")
	ETileDirection TileDirection;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Metrics")
	TMap<EConnection, bool> TileConnections;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (DisplayName = "Minimap"))
	UTexture2D* TileTexture;

	UFUNCTION()
	TArray<UGizmo*> GetGizmos(EGizmoType Type) const noexcept;

	UFUNCTION(BlueprintCallable)
	UTexture2D* GetTexture() { return TileTexture; }
	/**
	 * Unload the tile.
	 * Destroy each actors spawned on this tile.
	 */
	UFUNCTION()
	void Unload() const;
};
