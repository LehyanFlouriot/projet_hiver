#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjetHiver/MapGeneration/Tile.h"
#include "LevelGenerator.generated.h"

class ABaseBoss;
class ABaseNPC;
class ATileLimit;
class UDataTable;
struct FExplorationContractTileSet;

USTRUCT()
struct FFindNextLocationStruct
{
	GENERATED_BODY()

	bool HasSucceed;
	FVector2D FoundLocation;
};

UCLASS()
class PROJETHIVER_API ALevelGenerator : public AActor
{
	GENERATED_BODY()

public:
	ALevelGenerator();

#pragma region DataTable

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data Table")
	TObjectPtr<UDataTable> LevelsInfoDataTable;

	UFUNCTION(BlueprintCallable, Category = "Data Table")
	void ReadLevelDataFromTable();

#pragma endregion

public:
	UFUNCTION()
	void ResetLevel(bool bGameWon);
private:
	UFUNCTION()
	void ClearLayout();

#pragma region TileGeneration - All Clients and Server
public:
	UFUNCTION(BlueprintCallable, Category = "Tiles/Tile Spawning")
	ATile* SpawnTile(TSubclassOf<ATile> TileToSpawn, FTransform SpawnTransform);

	UFUNCTION(BlueprintCallable, Category = "Tiles/Generation")
	void GenerateTileLayout();

	UFUNCTION(BlueprintPure, Category = "Tiles/Layout")
	TSubclassOf<ATile> GetTileClass(ETileType TileType, TMap<EConnection, bool> TileConnections);

	UFUNCTION(BlueprintPure, Category = "Tiles/Utility")
	FTransform GetTileTransform(const FVector2D InTileLocation);

	UFUNCTION(Category = "Tiles/Utility")
	FFindNextLocationStruct FindNextLocation(FVector2D StartingLocation, EConnection WantedDirection);

	UFUNCTION(BlueprintPure, Category = "Tiles/Utility")
	bool IsTileUsed(const FVector2D Key);

	UFUNCTION(BlueprintCallable, Category = "Tiles/Layout")
	void ConnectTiles(FVector2D FromTile, FVector2D ToTile);

	UFUNCTION(BlueprintCallable, Category = "Tiles/Layout")
	void SetConnectionEnabled(FVector2D InTile, EConnection DirectionToAdd);

	UFUNCTION(BlueprintPure, Category = "Tiles/Utility")
	EConnection RevertConnection(EConnection InConnection);

	UFUNCTION(BlueprintPure, Category = "Tiles/Utility")
	bool AreConnectionsMatching(const TMap<EConnection, bool>& InConnections, const TMap<EConnection, bool>& AgainstConnections);

	UFUNCTION(BlueprintCallable, Category = "Tiles/Utility")
	TSubclassOf<ATile> FindRandomValidTile(TArray<TSubclassOf<ATile>>Array, TMap<EConnection, bool> inConnections);

	UFUNCTION(BlueprintCallable, Category = "Tiles/Generation")
	void GenerateExtraConnections();

	UFUNCTION(BlueprintPure, Category = "Tiles/Utility")
	TArray<FVector2D> GetLocationsAround(FVector2D inLocation);

	UFUNCTION(BlueprintPure, Category = "Tiles/Utility")
	bool AreTilesConnected(FVector2D inTile, FVector2D inTile2);

	UFUNCTION(BlueprintCallable, Category = "Tiles/Generation")
	void StepBack();

	UFUNCTION(BlueprintCallable, Category = "Tiles/Generation")
	void GenerateSpecialTiles();

	UFUNCTION(BlueprintCallable, Category = "Tiles/Generation")
	void GenerateExplorationContractTile();

	UFUNCTION(BlueprintCallable, Category = "Tiles/Generation")
	void GenerateStartTile();

	UFUNCTION(BlueprintCallable, Category = "Tiles/Generation")
	void GenerateEndTile();

	UFUNCTION(BlueprintCallable, Category = "Tiles/Utility")
	bool FindIfShouldStepBack();

	UFUNCTION(BlueprintCallable, Category = "Tiles/Generation")
	void AddNewTileToLayout(FVector2D FoundLocationInput);

	UFUNCTION(BlueprintCallable, Category = "Tiles/Generation")
	void GenerateTiles();

	UFUNCTION(BlueprintCallable, Category = "Tiles/Tile Spawning")
	void SpawnTiles();

	UFUNCTION(BlueprintCallable, Category = "Tiles/System")
	void GenerateStream();

	UFUNCTION()
	TArray<FLocatedTile> GenerateArrayFromLayoutMap();

	/** Adjacent tile positions */
	UFUNCTION()
	TArray<FVector2D> GetNeighborsPos(FVector2D TilePos);

	UFUNCTION()
	TArray<EConnection> GetConnections(FVector2D TilePos);

	UFUNCTION()
	FVector GetConnectionWorldLocation(FVector2D TilePos, EConnection Connection);

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Tile")
	TMap<FVector2D, FTileInformation> TileLayout;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> MinimapTexture;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Tile")
	FVector TileSize;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Tile")
	int32 TileAmount;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Tile")
	TSubclassOf<ATile> StartTile;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Tile")
	EConnection StartTileDirection;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Tile")
	TArray<TSubclassOf<ATile>> GenericTileList;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Tile")
	TArray<TSubclassOf<ATile>> SpecialTileList;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Tile")
	TArray<FExplorationContractTileSet> ExplorationContractTileSets;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Tile")
	int32 SpecialTilesAmount;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Tile")
	TArray<TSubclassOf<ATile>> EndTileList;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Tile")
	double ChanceForExtraConnections;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Tile")
	int32 MaxExtraConnections;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Utility")
	TMap<FVector2D, EConnection> Vector2DToDirection;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Utility")
	int32 Seed;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Utility")
	FRandomStream Stream;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Utility")
	TMap<EConnection, bool> EmptyConnections;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Tile")
	double StepBackChance;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Tile")
	TArray<FVector2D> StepBackHistoric;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Tile")
	FVector2D PreviousLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Utility")
	int32 CurrentLevel;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Tile")
	FVector2D StartTilePosition;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Tile")
	FVector2D EndTilePosition;

	/** Spawn the limit between two tiles */
	void SpawnLimit(FVector2D FromTile, FVector2D ToTile);

	/** 
	 * Array containing all the tiles limits.
	 * Je sais pas si c'est vraiment nécessaire, j'avais juste peur que le
	 * ramasse-miettes me supprime mes acteurs.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Tiles/Limit")
	TArray<ATileLimit*> TilesLimits;

	TSubclassOf<ATileLimit> TileLimitClass;

#pragma endregion
};

