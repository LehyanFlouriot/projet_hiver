#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "GameFramework/GameModeBase.h"
#include "Misc/Optional.h"
#include "EfhorisGameMode.generated.h"

class AAIDirector;
class AAIManager;
class UAkAudioEvent;
class AEfhorisGameState;
class AHealSeed;
class AInventoryCharacter;
class ALevelGenerator;
class APlayerCharacter;
class ATileLimit;
struct FItemInfo;
struct FItemStats;

struct FArmorKitRow;
struct FTrashItemRow;
struct FWeaponKitRow;
struct FGlovesRow;
struct FBeltRow;
struct FRingRow;
struct FNecklaceRow;
struct FEarringsRow;

class ULevelSequence;
class AAccumulationContract;
class AExplorationContract;
class AFailableContract;

UCLASS(minimalapi)
class AEfhorisGameMode : public AGameModeBase
{
	GENERATED_BODY()

	inline static FString SaveDirectory = "/SavedGames/";
	inline static FString GameSaveFileName = "Game.json";

protected:
	UPROPERTY(Transient)
	TArray<APawn*> PawnsToDeleteOnReset;

#pragma region Scaling
	UPROPERTY(EditAnywhere, Category = Scaling)
	float BuyValueScaling;

	UPROPERTY(EditAnywhere, Category = Scaling)
	float SellValueScaling;

	UPROPERTY(EditAnywhere, Category = Scaling)
	float PhysicalPowerScaling;

	UPROPERTY(EditAnywhere, Category = Scaling)
	float PhysicalDefenseScaling;

	UPROPERTY(EditAnywhere, Category = Scaling)
	float BlockingPhysicalDefenseScaling;

	UPROPERTY(EditAnywhere, Category = Scaling)
	float MagicalPowerScaling;

	UPROPERTY(EditAnywhere, Category = Scaling)
	float MagicalDefenseScaling;

	UPROPERTY(EditAnywhere, Category = Scaling)
	float BlockingMagicalDefenseScaling;

	UPROPERTY(EditAnywhere, Category = Scaling)
	float MaxHealthScaling;

	UPROPERTY(EditAnywhere, Category = Scaling)
	float MovementSpeedScaling;
#pragma endregion

#pragma region Random
	UPROPERTY(EditAnywhere, Category = Random)
	float PhysicalPowerRandom;

	UPROPERTY(EditAnywhere, Category = Random)
	float PhysicalDefenseRandom;

	UPROPERTY(EditAnywhere, Category = Random)
	float BlockingPhysicalDefenseRandom;

	UPROPERTY(EditAnywhere, Category = Random)
	float MagicalPowerRandom;

	UPROPERTY(EditAnywhere, Category = Random)
	float MagicalDefenseRandom;

	UPROPERTY(EditAnywhere, Category = Random)
	float BlockingMagicalDefenseRandom;

	UPROPERTY(EditAnywhere, Category = Random)
	float MaxHealthRandom;

	UPROPERTY(EditAnywhere, Category = Random)
	float MovementSpeedRandom;
#pragma endregion

#pragma region Contract
	UPROPERTY(EditAnywhere, Category = Contract)
	TArray<TSubclassOf<AAccumulationContract>> AccumulationContractTypes;

	UPROPERTY(EditAnywhere, Category = Contract)
	TArray<TSubclassOf<AExplorationContract>> ExplorationContractTypes;

	UPROPERTY(EditAnywhere, Category = Contract)
	TArray<TSubclassOf<AFailableContract>> FailableContractTypes;
#pragma endregion

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AInventoryCharacter> InventoryClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AHealSeed> HealSeedClass;

	UPROPERTY()
	TArray<AActor*> DroppedItems;

	virtual void BeginPlay() override;

	virtual void Logout(AController* Exiting) override;

	UFUNCTION()
	static int ScaleValueWithLevel(const int BaseValue, const int Level = 1, const float Coefficient = 0.25f);

	UFUNCTION()
	static float ScaleFValueWithLevel(const float BaseValue, const int Level = 1, const float Coefficient = 0.25f);

	UFUNCTION()
	static int RandomizeValueWithRange(const int BaseValue, const float Range);

	UFUNCTION()
	static float RandomizeFValueWithRange(const float BaseValue, const float Range);

	UFUNCTION()
	void ScaleItemInfo(FItemInfo& ItemInfo) const;

	UFUNCTION()
	void RandomizeItemInfo(FItemInfo& ItemInfo) const;

	/** Reset Dedicated Serv */
	UFUNCTION()
	void ResetDedicatedServ();

	UFUNCTION()
	void RebuildLevel(bool bGameWon, bool bShouldSave = true);

	UFUNCTION()
	void SaveGame(bool bIsAsynch = false) const;

	UFUNCTION()
	void GenerateContracts();

	UFUNCTION()
	void GenerateAccumulationContract(AEfhorisGameState* EfhorisGameState);

	UFUNCTION()
	void GenerateExplorationContract(AEfhorisGameState* EfhorisGameState);

	UFUNCTION()
	void GenerateFailableContract(AEfhorisGameState* EfhorisGameState);

	UFUNCTION()
	void CheckValidateContracts();

	UFUNCTION()
	bool CheckValidateAccumulationContract(AEfhorisGameState* EfhorisGameState);

	UFUNCTION()
	bool CheckValidateExplorationContract(AEfhorisGameState* EfhorisGameState);

	UFUNCTION()
	bool CheckValidateFailableContract(AEfhorisGameState* EfhorisGameState);

public:
	friend class UEfhorisCheatManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, NoClear)
	TSubclassOf<ALevelGenerator> LevelGeneratorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,NoClear)
	TSubclassOf<AAIDirector> AIDirectorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, NoClear)
	TSubclassOf<AAIManager> AIManagerClass;

	// Store an item into the shared inventory
	UFUNCTION(BlueprintCallable)
	void StoreItem(APickableItem* Item);

	// Store an item info into the shared inventory
	UFUNCTION(BlueprintCallable)
	void StoreItemInfo(FItemInfo Item);

	// Store an item into the shared inventory
	UFUNCTION(BlueprintCallable)
	void RemoveItemInfo(FItemInfo Item);

	UFUNCTION()
	void EquipKitToPlayer(APlayerCharacter* Character, FItemInfo ItemInfo);

	UFUNCTION()
	void UnequipKitFromPlayer(APlayerCharacter* Character, FItemInfo ItemInfo);

	UFUNCTION()
	void SetSmithItemToPlayer(APlayerCharacter* Character, FItemInfo ItemInfo);

	UFUNCTION()
	void RemoveSmithItemFromPlayer(APlayerCharacter* Character);

	UFUNCTION()
	void SmithPlayerItem(APlayerCharacter* Player, int LevelOffset = 1);

	UFUNCTION()
	void TryHealAllPlayers();

	UFUNCTION()
	void TryBuyItem(AMerchant* Merchant, FItemInfo ItemInfo, const APlayerCharacter* Player);

	UFUNCTION()
	void TrySellItem(AMerchant* Merchant, FItemInfo ItemInfo, const APlayerCharacter* Player);

	template <class T>
	UFUNCTION(BlueprintCallable)
	T* SpawnActor(UClass* Class, const FTransform& Transform = FTransform{}, const FActorSpawnParameters& SpawnParameters = FActorSpawnParameters{})
	{
		return GetWorld()->SpawnActor<T>(Class, Transform, SpawnParameters);
	}

	template <class T>
	UFUNCTION(BlueprintCallable)
	T* SpawnActorDeferred(UClass* Class, FTransform const& Transform, AActor* Owner = nullptr, APawn* Instigator = nullptr,
						  ESpawnActorCollisionHandlingMethod CollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined)
	{
		return GetWorld()->SpawnActorDeferred<T>(Class, Transform, Owner, Instigator, CollisionHandlingOverride);
	}

	UFUNCTION()
	APickableItem* SpawnArmorKit(const FName& RowName, const FTransform& Transform, int Level = 1, bool bRandomizeStats = false);

	UFUNCTION()
	APickableItem* SpawnTrashItem(const FName& RowName, const FTransform& Transform, int Level = 1);

	UFUNCTION()
	APickableItem* SpawnWeaponKit(const FName& RowName, const FTransform& Transform, int Level = 1, bool bRandomizeStats = false);

	UFUNCTION()
	AHealSeed* SpawnHealSeed(const FTransform& Transform);

	UFUNCTION()
	APickableItem* SpawnBelt(const FName& RowName, const FTransform& Transform, int Level = 1, bool bRandomizeStats = false);

	UFUNCTION()
	APickableItem* SpawnGloves(const FName& RowName, const FTransform& Transform, int Level = 1, bool bRandomizeStats = false);

	UFUNCTION()
	APickableItem* SpawnNecklace(const FName& RowName, const FTransform& Transform, int Level = 1, bool bRandomizeStats = false);

	UFUNCTION()
	APickableItem* SpawnEarrings(const FName& RowName, const FTransform& Transform, int Level = 1, bool bRandomizeStats = false);

	UFUNCTION()
	APickableItem* SpawnRing(const FName& RowName, const FTransform& Transform, int Level = 1, bool bRandomizeStats = false);

	FItemInfo GenerateArmorKitInfo(const FName& RowName, TOptional<FArmorKitRow> Row, int Level = 1, bool bRandomizeStats = false) const;

	FItemInfo GenerateTrashItemInfo(const FName& RowName, TOptional<FTrashItemRow> Row, int Level = 1) const;

	FItemInfo GenerateWeaponKitInfo(const FName& RowName, TOptional<FWeaponKitRow> Row, int Level = 1, bool bRandomizeStats = false) const;

	FItemInfo GenerateGlovesInfo(const FName& RowName, TOptional<FGlovesRow> Row, int Level = 1, bool bRandomizeStats = false) const;

	FItemInfo GenerateBeltInfo(const FName& RowName, TOptional<FBeltRow> Row, int Level = 1, bool bRandomizeStats = false) const;

	FItemInfo GenerateRingInfo(const FName& RowName, TOptional<FRingRow> Row, int Level = 1, bool bRandomizeStats = false) const;

	FItemInfo GenerateNecklaceInfo(const FName& RowName, TOptional<FNecklaceRow> Row, int Level = 1, bool bRandomizeStats = false) const;

	FItemInfo GenerateEarringsInfo(const FName& RowName, TOptional<FEarringsRow> Row, int Level = 1, bool bRandomizeStats = false) const;

	UFUNCTION()
	void WinGame();

	UFUNCTION()
	void TryLoseGame();

	UFUNCTION()
	void ReloadGame();

	UFUNCTION()
	void CallInventory(APlayerCharacter* PlayerCharacter) const;

	UFUNCTION()
	void AddPawnToDeleteOnLevelReset(APawn* Pawn);

	/** Update visited tiles and AI Director */
	UFUNCTION()
	void TileChanged(ATileLimit* Limit, FVector2D PlayerPos) const;

	/** Add amount of damage taken by players */
	UFUNCTION()
	void AddDamageTaken(double Damage) const;

	//Health panel
	UFUNCTION()
	void RefreshHealthPanel(const TArray<FPlayerInfo>& Descriptions);

	UFUNCTION()
	void RefreshMapNicknames(const TArray<FPlayerInfo>& Descriptions);

	UFUNCTION()
	void StartBoss();

	UFUNCTION(BlueprintCallable)
	void EnterBossCombat();

	UFUNCTION()
	void PlaySequence(ULevelSequence* SequenceToPlay, bool bShouldPlayStartSequenceAfter = false);

	UFUNCTION()
	void PlayMusic(UAkAudioEvent* MusicToPlay);

	UFUNCTION()
	void StopAllMusic();

	UFUNCTION()
	void SetMusicState(FName StateGroup, FName NewState);
};
