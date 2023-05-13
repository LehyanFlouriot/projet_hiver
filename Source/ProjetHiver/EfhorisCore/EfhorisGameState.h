#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ProjetHiver/Items/ItemInfo.h"
#include "ProjetHiver/Items/ItemStats.h"
#include "ProjetHiver/Contracts/AccumulationContract.h"
#include "ProjetHiver/Contracts/ExplorationContract.h"
#include "ProjetHiver/Contracts/FailableContract.h"
#include "EfhorisGameState.generated.h"

class APickableItem;
class ALevelGenerator;
class AAIDirector;
class AAIManager;
class AInventoryCharacter;
class AEfhorisPlayerState;
struct FGameSave;
struct FLocatedTile;

USTRUCT(BlueprintType)
struct FPlayersAverageEquipment
{
	GENERATED_BODY()

	FPlayersAverageEquipment() = default;
	FPlayersAverageEquipment(const float _AverageArmorLevel, const float _AverageWeaponLevel,
							 const FItemStats _AverageArmorStats, const FItemStats _AverageWeaponStats);

	float AverageArmorLevel;
	float AverageWeaponLevel;

	FItemStats AverageArmorStats;
	FItemStats AverageWeaponStats;
};

UCLASS()
class PROJETHIVER_API AEfhorisGameState : public AGameStateBase
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	FGuid GameId;

	UPROPERTY()
	FString GameName;

	UPROPERTY()
	bool bHasBeenCreated;

	UPROPERTY(Transient)
	ALevelGenerator* LevelGenerator;

	UPROPERTY(Transient)
	AAIDirector* AIDirector;

	UPROPERTY(Transient)
	AAIManager* AIManager;

	UPROPERTY(Replicated, BlueprintReadOnly, ReplicatedUsing=OnRep_SharedInventoryRep)
	TArray<FItemInfo> SharedInventory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, ReplicatedUsing=OnRep_MoneyRep)
	int Money;

	UPROPERTY(Transient)
	mutable AEfhorisPlayerState* Creator;

	UPROPERTY(Transient, Replicated, ReplicatedUsing=OnRep_ContractRep)
	AAccumulationContract* AccumulationContract;

	UPROPERTY(Transient, Replicated, ReplicatedUsing=OnRep_ContractRep)
	AExplorationContract* ExplorationContract;

	UPROPERTY(Transient, Replicated, ReplicatedUsing=OnRep_ContractRep)
	AFailableContract* FailableContract;

	UPROPERTY(Replicated, Transient, ReplicatedUsing=OnRep_LayoutMinimapRep)
	TArray<FLocatedTile> LayoutMinimap;
public:
	void GenerateLayoutMinimap();

	UFUNCTION()
	void OnRep_LayoutMinimapRep() const;

protected:
#pragma region Medic
	UPROPERTY(Transient, Replicated, ReplicatedUsing=OnRep_MedicRep)
	int HealAllPlayersCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Medic, BlueprintReadWrite)
	int HealAllPlayerInitialCost = 500;
#pragma endregion

#pragma region Smith

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int BaseCost = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CostPerLevelMultiplier = 2;
#pragma endregion

public:
	AEfhorisGameState();

	virtual void Tick(float DeltaSeconds) override;

	// Store an item into the shared inventory, called only by gamemode
	UFUNCTION(BlueprintCallable)
	void StoreItem(APickableItem* Item);

	UFUNCTION(BlueprintCallable)
	void StoreItemInfo(FItemInfo ItemInfo);

	// Store an item into the shared inventory, called only by gamemode
	UFUNCTION(BlueprintCallable)
	void RemoveItemInfo(FItemInfo Item);

	UFUNCTION()
	bool ContainsItem(const FItemInfo& Item) const;

	UFUNCTION(BlueprintCallable)
	int GetHealAllPlayersCost() const noexcept { return HealAllPlayersCost; }

	UFUNCTION(BlueprintCallable)
	int GetSmithCost(FItemInfo ItemInfo) const noexcept { return BaseCost * ItemInfo.Level * (ItemInfo.Level == 1 ? 1 : CostPerLevelMultiplier); }

	UFUNCTION(BlueprintCallable)
	void DoubleHealCost() { HealAllPlayersCost *= 2; }

	UFUNCTION(BlueprintCallable)
	void ResetHealCost() { HealAllPlayersCost = HealAllPlayerInitialCost; }

	UFUNCTION()
	void IsTargetedByInventory(const AEfhorisPlayerState* Player);

protected:
	UFUNCTION()
	void RefreshUI() const;

public:
	UFUNCTION()
	void OnRep_SharedInventoryRep() const;

	UFUNCTION()
	void OnRep_MoneyRep() const;

	UFUNCTION()
	void OnRep_ContractRep() const;

	UFUNCTION()
	void OnRep_MedicRep() const;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsGameEnded() const noexcept { return bGameEnded; };

	UFUNCTION()
	void SetGameEnded(const bool Completed) noexcept { bGameEnded = Completed; }

    UFUNCTION()
	void SetLevelGenerator(ALevelGenerator* LvlGen) noexcept { LevelGenerator = LvlGen; }

	UFUNCTION()
	ALevelGenerator* GetLevelGenerator() const noexcept { return LevelGenerator; }

	UFUNCTION()
	int32 GetCurrentLevel() const noexcept;

	UFUNCTION()
	bool IsLevelGeneratorValid() const;

	UFUNCTION()
	AInventoryCharacter* GetInventory() const noexcept;

	UFUNCTION()
	void SetInventory(AInventoryCharacter* Inventory_) noexcept;

	/* ------------ AI Director ------------ */

	AAIDirector* GetAIDirector() const noexcept { return AIDirector; }

	void SetAIDirector(AAIDirector* AIDirector_) noexcept { AIDirector = AIDirector_; }

	/* ------------ AI Manager ------------ */

	AAIManager* GetAIManager() const noexcept { return AIManager; }

	void SetAIManager(AAIManager* AIManager_) noexcept { AIManager = AIManager_; }

	/* ------------ Measures ------------ */

	UFUNCTION(BlueprintCallable)
	/** Amount of damage taken by all the players. */
	float GetDamageTaken() const noexcept { return DamageTaken; }

	/** Add damage to the amount of damage taken. */
	void AddDamageTaken(long double DamageToAdd) noexcept { DamageTaken += DamageToAdd; }

	UFUNCTION(BlueprintCallable)
	/** Number of player in the session. */
	int GetNumberPlayer() const noexcept { return PlayerArray.Num(); }

	UFUNCTION(BlueprintCallable)
	/** Number of player alive in the session */
	int GetNumberPlayerAlive() const;

	UFUNCTION(BlueprintCallable)
	/** Average weapon level of all players */
	FPlayersAverageEquipment GetAveragePlayersEquipment();

	/** Time since last levelReset */
	UFUNCTION(BlueprintCallable)
	float GetLevelTimer() const;

	UFUNCTION(BlueprintCallable)
	void Authority_ResetLevelTimer();

	/** Increment number of tile visited. */
	void IncrementNbVisitedTiles() noexcept { ++NbVisitedTiles; }

	/** Number of tile visited. */
	int GetNbVisitedTiles() const noexcept { return NbVisitedTiles; }

	/** Sum of max health for each player */
	UFUNCTION(BlueprintCallable)
	float GetPlayersMaxHealth() const noexcept;

	/** Sum of current health for each player */
	UFUNCTION(BlueprintCallable)
	float GetPlayersCurrentHealth() const noexcept;

	/* ------------ End Measures ------------ */

	const FGuid& GetGameId() const noexcept { return GameId; }
	void SetGameId(const FGuid& NewGameId) { GameId = NewGameId; }

	FString GetGameName() const noexcept;
	void SetGameName(const FString& GameName_) noexcept { GameName = GameName_; }

	AEfhorisPlayerState* GetCreator() const noexcept { return Creator; }
	void SetCreator(AEfhorisPlayerState* PlayerCreator) const { Creator = PlayerCreator; }

	FString GetGameCreatorId() const noexcept;
	void SetGameCreatorId(const FString& NewGameCreatorId) noexcept;

	bool HasBeenCreated() const noexcept { return bHasBeenCreated; }
	void SetHasBeenCreated(const bool _bHasBeenCreated) noexcept { bHasBeenCreated = _bHasBeenCreated; }

	const TArray<FItemInfo>& GetSharedInventory() const noexcept { return SharedInventory; }

	UFUNCTION(BlueprintCallable)
	int GetMoney() const noexcept { return Money; }
	void SetMoney(const int _Money) noexcept;

	/* ------------ Save ------------ */

	/**
	 * Get gamestate save.
	 *
	 * @param GameState, the gamestate
	 * @return the object containing every relevant state to save GameState
	 */
	FGameSave GetGameStateSave() const noexcept;

	/**
	 * Load a gamestate save.
	 *
	 * @param GameSave, the object containing every relevant state to save GameState
	 */
	void SetGameStateSave(FGameSave& GameSave) noexcept;

#pragma region Contracts
	UFUNCTION(BlueprintCallable)
	AAccumulationContract* GetAccumulationContract() const noexcept;
	void SetAccumulationContract(AAccumulationContract* Contract) noexcept;

	UFUNCTION(BlueprintCallable)
	AExplorationContract* GetExplorationContract() const noexcept;
	void SetExplorationContract(AExplorationContract* Contract) noexcept;

	UFUNCTION(BlueprintCallable)
	AFailableContract* GetFailableContract() const noexcept;
	void SetFailableContract(AFailableContract* Contract) noexcept;

#pragma endregion

protected:
	UPROPERTY(Replicated)
	bool bGameEnded = false;

	UPROPERTY(Replicated)
	AInventoryCharacter* Inventory = nullptr;

	UPROPERTY(Transient)
	float DamageTaken = 0.f;

	UPROPERTY(Transient)
	int NbVisitedTiles = 0;

	UPROPERTY(Replicated)
	float LevelStartTime;

	UPROPERTY(Transient)
	float TimeContractStartTime;

	UPROPERTY(Transient)
	bool bTimeContractStarted;
};
