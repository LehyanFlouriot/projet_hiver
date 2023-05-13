#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ProjetHiver/Save/FilesToSave.h"
#include "ProjetHiver/Save/SaveRunnable.h"
#include "SaveSubsystem.generated.h"

class AEfhorisGameState;
class AEfhorisPlayerState;

USTRUCT()
struct PROJETHIVER_API FPlayerSaves {
	GENERATED_BODY()

	UPROPERTY()
	TArray<FString> PlayersId{};

	UPROPERTY()
	TArray<FPlayerSave> PlayerSaves{};

	void Add(const FString& PlayerId, FPlayerSave PlayerSave) noexcept {
		PlayersId.Add(PlayerId);
		PlayerSaves.Add(PlayerSave);
	}

	FPlayerSave Get(const FString& PlayerId) const noexcept {
		if (int Index; PlayersId.Find(PlayerId, Index))
			return PlayerSaves[Index];

		return FPlayerSave();
	}
};

/** Save complete delegate */
DECLARE_DELEGATE(FSaveCompleteDelegate);

/**
 * Save Subsytem
 */
UCLASS()
class PROJETHIVER_API USaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	// Utils string for paths
	FString SaveDirectory = "SavedGames/";
	FString GameSaveFileName = "Game.json";
	FString DirectorSaveFileName = "AI.json";
	FString Extension = ".json";

	// Every game saves loaded from disk
	TArray<FGameSave> GameSaves;

	// Synchronizer for save's threads
	FEventRef EventSaveAsynch;

	// Queue files to saves
	TQueue<FFilesToSave, EQueueMode::Spsc> FilesToSaveQueue;

	//Pointer on the Thread who save the game
	TUniquePtr<FSaveRunnable> SaveThread;

	/** 
	 * Every player's save associated to the loaded game.
	 * The key is the steam id, and the value is the player's save associated.
	 * 
	 */
	TMap<FString, FPlayerSave> PlayerSaves;

	/** Delegate an action on save complete */
	FSaveCompleteDelegate SaveCompleteDelegate;

	/** Auto save activated */
	bool bIsAutoSaveActivated = true;

public:
	USaveSubsystem();

	/**
	 * Save the current state of the game.
	 * 
	 * @param bIsAsynch, is the save asynchronous
	 */
	void SaveGame(bool bIsAsynch = false);

	/**
	 * Load a game from a game save.
	 * Set up the game instance attributs for loading.
	 *
	 * @param GameId, id of the game
	 */
	void LoadGame(const FString& GameId) const;

	/**
	 * Get all saves on folder.
	 * 
	 * @return the GameState's saves on folder
	 */
	const TArray<FGameSave>& GetAllSaves();

	/**
	 * Delete a save from the disk.
	 * 
	 * @param GameSave, the game save to delete
	 */
	void DeleteSave(const FGameSave& GameSave);

	/**
	 * Write the states saved into the files corresponding.
	 *
	 * @param FilesToSave, the files to save with util data associated
	 */
	void WriteStatesSave(const FFilesToSave& FilesToSave) const;

	/**
	 * Write the states saved into the files corresponding asynchronously.
	 *
	 * @param FilesToSave, the files to save with util data associated
	 */
	void WriteFilesSaveAsync(const FFilesToSave& FilesToSave);

	/**
	 * Store the states saved into the data base.
	 * 
	 * @param FilesToSave, the files to save with util data associated
	 */
	void StoreStatesSave(const FFilesToSave& FilesToSave) const;

	/**
	 * Get the state associated to a player.
	 * 
	 * @param PlayerId, the steam id of the player
	 * @param PlayerSave, the state associated to this steam account
	 * @return if a save has been find
	 */
	bool GetPlayerSave(const FString& PlayerId, FPlayerSave& PlayerSave) const;

	/**
	 * Store on the map the player saves given in parameter.
	 * Allow the creator to store his internal saves on the server.
	 * 
	 * @param PlayerSaves_, the player saves to store
	 */
	void StorePlayerStateSaves(const FPlayerSaves& PlayerSaves_);

	/**
	 * Reset the loaded saves in case of reset of dedicated server.
	 */
	void ResetSaves() { PlayerSaves.Empty(); GameSaves.Empty();	}

	/**
	 * Init the saver thread.
	 */
	void InitSaverThread() const;

	/**
	 * Stop the saver thread.
	 */
	void StopSaverThread() const;

	/**
	 * Get the save complete delegate to bind action at the end of the save.
	 */
	FSaveCompleteDelegate& GetSaveCompleteDelegate() noexcept { return SaveCompleteDelegate; }

	// Option auto save methods
	bool IsAutoSaveActivated() const noexcept { return bIsAutoSaveActivated; }
	void ToggleAutoSave() noexcept { bIsAutoSaveActivated = !bIsAutoSaveActivated; }
	void SetAutoSave(bool bIsActivated) noexcept { bIsAutoSaveActivated = bIsActivated; }

private:
	/**
	 * Get the structs of states to save with util data associated.
	 * 
	 * @param bCreatorValid, indicate if the creator is valid 
	 */
	FFilesToSave GetFilesToSave(bool& bCreatorValid);

	/**
	 * Save the game localy.
	 * Write save files on the creator disk.
	 * 
	 * @param FilesToSave, the files to save with util data associated
	 * @param bIsAsynch
	 */
	void SaveGameDisk(const FFilesToSave& FilesToSave, bool bIsAsynch);

	/**
	 * Save the game on the data base.
	 * Store save files on the data base.
	 * 
	 * @param FilesToSave, the files to save with util data associated
	 * @param bIsAsynch
	 */
	void SaveGameDataBase(const FFilesToSave& FilesToSave, bool bIsAsynch);

	/**
	 * Get the path and the save struct of all player.
	 * 
	 * @param EfhorisGameState, the game state
	 * @return the array of playerstate's saves and their path
	 */
	TArray<FPlayerToSave> GetPlayerStatesToSave(const AEfhorisGameState* EfhorisGameState);

	/**
	 * Get the map of player saves sored on disk.
	 * Should only be called by the creator on client side.
	 *
	 * @param GameId, the current game id
	 * @return the player saves found in the folder
	 */
	FPlayerSaves LoadPlayerSaves(const FString& GameId) const;

	/**
	 * Load the director save of a given game id.
	 *
	 * @param GameId, the id of game of the director save searched
	 */
	FDirectorSave LoadDirectorSave(const FString& GameId) const;

	/**
	 * Write the state saved into the file corresponding.
	 * Template method, struct of object in parameter.
	 *
	 * @param Path, the path of the file
	 * @param Save, the object containing every relevant state to save
	 * @return the serialized save struct
	 */
	template <class T>
	FString WriteStateSave(const FString& Path, const T& Save) const;

	/**
	 * Read the state saved from the file corresponding.
	 * Template method, the struct of returned object.
	 *
	 * @param Path, the path of the file
	 * @return the object containing every relevant state to save
	 */
	template <class T>
	T ReadStateSave(const FString& Path) const;
};
