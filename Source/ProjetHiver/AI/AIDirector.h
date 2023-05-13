#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/AI/AIDirectorStatesMachine.h"
#include "ProjetHiver/AI/Utility/AIDirectorUtilityTheory.h"
#include "ProjetHiver/AI/ActorsByGizmo.h"
#include "ProjetHiver/Random/WeightedRandomSelector.h"
#include "ProjetHiver/Random/UniformSelector.h"
#include "AIDirector.generated.h"

class AAIManager;
class ABaseNPC;
class ALootChest;
class AEfhorisGameState;
class ALevelGenerator;
class UInfluenceMapManager;
class UDataTable;
class UGizmo;
class ATile;
enum class EGizmoType : uint8;
struct FDirectorSave;
struct FGizmosSave;
struct FGizmoSave;
struct FMercenarySave;
struct FKevinSave;
struct FBossSave;
struct FAloneEnemySave;
struct FEnemySave;
struct FChestSave;
struct FTileInformation;

enum EAIDifficulty
{
	Low,	//0-20
	Medium,	//20-50
	High,	//50-80
	Insane,	//80-100
};

struct FSpawnActorAsynchInfo {
	FVector2D AssociatedTile;
	UGizmo* Gizmo;
	TSubclassOf<AActor> ActorClass;
	FTransform Transform;
	FSpawnActorAsynchInfo(FVector2D Tile, UGizmo* Giz, TSubclassOf<AActor> Class, FTransform Trans)
		: AssociatedTile{ Tile }, Gizmo { Giz }, ActorClass{ Class }, Transform{ Trans } {
	}
};

/**
 * Efhoris implementation of AI Director.
 * Spawn Mobs and Create Groups.
 */
UCLASS(Blueprintable)
class PROJETHIVER_API AAIDirector final : public AInfo
{
	GENERATED_BODY()

	/** Utility Computer. */
	FAIDirectorUtilityTheory Utilities;

	/** Current Difficulty */
	EAIDifficulty Difficulty;

	/** State Computer */
	FAIDirectorFSM AIDirectorFSM;

	/** Loaded Tiles' Positions */
	TArray<FVector2D> LoadedTiles;

	/** Information to define Intensity of the party */
	UPROPERTY(EditAnywhere)
	UDataTable* IntensityDataTable;

	/** Random Selector with weight function */
	FWeightedRandomSelector RandomSelector;

	/** Blueprint spawnable for each gizmos */
	UPROPERTY(EditAnywhere)
	TArray<FActorsByGizmo> GizmosActors;

	/** Random Selector with weight function */
	FUniformSelector<int> UniformSelector;

	UPROPERTY(EditAnywhere)
	int NbOfActorsToSpawnEachFrame;

	/** Array of Actors to spawn on asynchronously */
	TArray<FSpawnActorAsynchInfo> ActorToSpawnInfos;

	UPROPERTY(Transient)
	AAIManager* Manager;

	UPROPERTY(Transient)
	ALevelGenerator* LvlGenerator;

	bool bCheatDifficultyActivated = false;
	
public:
	friend class UEfhorisCheatManager;

	AAIDirector();

	/** 
	 * Update AI Director.
	 * Called for each tile change.
	 * 
	 * @param bIsFirstUpdate
	 */
	void Update(bool bIsFirstUpdate = false);

	EAIDifficulty GetDifficulty() const noexcept { return Difficulty; }

	FAIDirectorFSM GetStateMachine() const noexcept { return AIDirectorFSM; }

	/**
	 * Used as a cheat to modify the difficulty
	 */
	void SetDifficulty(int Difficulty_) noexcept;

	/**
	 * Used to modify the intensity
	 */
	void SetIntensity(int Intensity_) noexcept { AIDirectorFSM.SetIntensity(Intensity_); };

	void SetAIManager(AAIManager* Manager_) noexcept { Manager = Manager_; }

	void SetLvlGenerator(ALevelGenerator* LevelGenerator) noexcept { LvlGenerator = LevelGenerator; }

	void ResetDirector() noexcept;

	void SetCurrentNumberOfRoomVisited(uint8 NbOfRoomsVisited) noexcept { AIDirectorFSM.SetNumberOfRoomsVisited(NbOfRoomsVisited); }
	
	TArray<FVector2D> GetLoadedTiles() const noexcept { return LoadedTiles; }

	TArray<FSpawnActorAsynchInfo> GetActorToSpawnInfos() const noexcept { return ActorToSpawnInfos; }

	// Save 

	FDirectorSave GetAIDirectorSave() const;

	void SetAIDirectorSave(FDirectorSave& DirectorSave);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Call every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere, NoClear)
	TSubclassOf<ABaseNPC> GenericEnemy;

	int ComputeSpawnRate(EGizmoType Type) const;

	/**
	 * Spawn actors on available gizmos.
	 *
	 * @param TilesToLoad tiles which should be loaded
	 */
	void SpawnGizmos(const TArray<FVector2D>& TilesToLoad);

	/**
	 * Spawn actors on start tile.
	 */
	void SpawnStartTile();

	/**
	 * Spawn gizmos of the good type.
	 *
	 * @param Type type of gizmo
	 * @param Tile tile which should be spawn
	 * @param bIsStartTile, spawning actors on start tile
	 */
	void SpawnGizmoFromTile(EGizmoType Type, const FVector2D& Tile, bool bIsStartTile = false);

	/**
	 * Spawn actor on a gizmo.
	 * 
	 * @param Gizmo on which spawn an actor
	 * @param bIsStartTile, spawning actors on start tile
	 */
	void SpawnGizmo(UGizmo* Gizmo, bool bIsStartTile, const FVector2D& Tile);

	/** Spawn an actor on a gizmo's transform. */
	AActor* SpawnActorOnGizmo(TSubclassOf<AActor> ActorClass, FTransform Transform) const;

	void UpdateDifficulty();

	/**
	 * Update loaded tiles and to load tiles.
	 * Unload tiles which are too far from players,
	 * Load tiles wich are close from players.
	 *
	 * @return positions of tiles to load
	 */
	TArray<FVector2D> UpdateLoadedTiles();

	/** Players' normalised positions. */
	TArray<FVector2D> GetPlayersPos(AEfhorisGameState* GameState) const;

	/** Tiles which should be loaded. */
	TArray<FVector2D> GetPlayersAdjacentTiles(AEfhorisGameState* GameState) const;

	/** Actions to do when a new tile is visited. */
	void OnNewVisitedTile(FTileInformation& TileInfo, AEfhorisGameState* GameState) const;

	/** Unload tiles too far. */
	void UnloadTiles(int Index, int Count);

	/**
	 * Generate the save of every actors spawn on a gizmo.
	 * 
	 * @param Tile, save every spawned gizmo of Tile
	 * @param Tile2D, the coordonates of the tile
	 */
	FGizmosSave GetGizmosSave(const ATile* Tile, const FVector2D& Tile2D) const;

	// Je vomis
	TArray<FChestSave> GetChestsSave(const ATile* Tile, const FVector2D& Tile2D) const;
	TArray<FAloneEnemySave> GetEnemiesSave(const ATile* Tile, const FVector2D& Tile2D) const;
	TArray<FEnemySave> GetSubBossesSave(const ATile* Tile, const FVector2D& Tile2D) const;
	TArray<FBossSave> GetBossesSave(const ATile* Tile, const FVector2D& Tile2D) const;
	TArray<FKevinSave> GetKevinsSave(const ATile* Tile, const FVector2D& Tile2D) const;
	TArray<FMercenarySave> GetMercenariesSave(const ATile* Tile, const FVector2D& Tile2D) const;
	TArray<FGizmoSave> GetOtherGizmosSave(const ATile* Tile, const FVector2D& Tile2D, EGizmoType Type) const;

	/**
	 * Spawn each actors loaded to gizmo corresponding.
	 *
	 * @param GizmosSave, gizmos saved
	 * @param Tile, save every spawned gizmo of Tile
	 * @param Tile2D, the coordonates of the tile
	 */
	void SetGizmosSave(FGizmosSave& GizmosSave, const ATile* Tile, const FVector2D& Tile2D);

	UGizmo* FindAssociatedGizmo(const FVector& SaveTile, const FVector2D& Tile2D, const ATile* Tile, const FString& GizmoName, EGizmoType Type) const;

	// Je gerbe
	void SetChestsSave(const TArray<FChestSave>& ChestsSave, const ATile* Tile, const FVector2D& Tile2D);
	void SetEnemiesSave(TArray<FAloneEnemySave>& EnemiesSave, const ATile* Tile, const FVector2D& Tile2D);
	void SetSubBossesSave(TArray<FEnemySave>& SubBossesSave, const ATile* Tile, const FVector2D& Tile2D);
	void SetBossesSave(const TArray<FBossSave>& BossesSave, const ATile* Tile, const FVector2D& Tile2D);
	void SetKevinsSave(const TArray<FKevinSave>& KevinsSave, const ATile* Tile, const FVector2D& Tile2D);
	void SetMercenariesSave(const TArray<FMercenarySave>& MercenariesSave, const ATile* Tile, const FVector2D& Tile2D);
	void SetOtherGizmosSave(const TArray<FGizmoSave>& OtherGizmosSave,const ATile* Tile, const FVector2D& Tile2D, EGizmoType Type);
};
