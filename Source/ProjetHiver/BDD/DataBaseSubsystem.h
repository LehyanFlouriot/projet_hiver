#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "DataBaseSubsystem.generated.h"

class UFileHelperSubsystem;
struct FUserScoreboard;
enum class EUserScoreboardStatistic : uint8;

/**
 * Util user's infos.
 */
USTRUCT()
struct PROJETHIVER_API FUserInfos {
	GENERATED_BODY()

	UPROPERTY()
	FString GuestToken;

	UPROPERTY()
	FString SessionToken;

	operator bool() const {
		return !GuestToken.IsEmpty() || !SessionToken.IsEmpty();
	}
};

/**
 * Data Base Subsystem.
 */
UCLASS()
class PROJETHIVER_API UDataBaseSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	// Utils string for paths
	FString DataDirectory = "DataBaseInfos/";
	FString FileName = "Data.json";
	FString ServerPort = "5000";
	FString ServerURL;

	/* Http Module */
	FHttpModule* Http;

	/* File helper subsystem */
	UFileHelperSubsystem* FileSubsystem;

	/** The user's steamId used as userId by the DataBase. */
	FString SteamId;
	FString SteamIdEncoded;
	/** The util data base's user's infos. */
	FUserInfos Infos;

	/** Defined if the player is logged. */
	bool bIsLoggedIn = false;
	
public:
	/**
	 * Struct representation of a save for facilating storing on data base.
	 */
	struct FDataBaseSave {
		/* Game Id */
		FString GameId;

		/* Game Save */
		FString GameSave;

		/* Director Save */
		FString DirectorSave;

		/* Map of Key : PlayerId, Value : Player Save */
		TMap<FString, FString> PlayerSaves;
	};

private:
	/* The current save to store */
	mutable FDataBaseSave DataSave;

public:
	const FUserInfos& GetUserInfos() const noexcept { return Infos; }
	void SetUserInfos(const FUserInfos& Infos_) noexcept { Infos = Infos_; }
	const FString& GetSteamId() const noexcept { return SteamId; }

	/**
	 * Initialize the user from his steamId.
	 * 
	 *  @param SteamId_, steam id of the player
	 */
	void Init(const FString& SteamId_);

	/**
	 * Is the player logged in ?
	 */
	bool IsLoggedIn() const noexcept { return bIsLoggedIn; }

	/**
	 * Store a save into the data base associated to this player.
	 * 
	 * @param Save, the save to store
	 */
	void StoreSave(const FDataBaseSave& Save);

	/**
	 * Load a director save associated to a game id from the data base.
	 * 
	 * @param GameId, the game id
	 */
	void LoadDirectorSave(const FString& GameId);

	/**
	 * Load a player saves associated to a game id from the data base.
	 *
	 * @param GameId, the game id
	 */
	void LoadPlayerSaves(const FString& GameId);

	/**
	 * Delete a save from the data base.
	 * 
	 * @param GameId, the game id to delete
	 */
	void DeleteSave(const FString& GameId);

	/**
	 * Get all saves from data base.
	 */
	void GetAllSaves();

	/**
	 * Gets the scoreboard corresponding to the player id.
	 *
	 * @param PlayerId, the player id
	 */
	void GetPlayerScoreboard(const FString& PlayerId);

	/**
	 * Increase a statistic of a player by an amount.
	 *
	 * @param PlayerId, the player id
	 * @param Scoreboard, the scoreboard stats to store
	 */
	void IncreaseStatistics(const FString& PlayerId, const FUserScoreboard& Scoreboard);

private:
	/**
	 * Login a player to the data based.
	 * Create an user if needed.
	 * Create or refresh a session id needed.
	 */
	void Login();
	void OnPostLogin();

	/**
	 * Verify if the user still exists.
	 * If the user does not exist it is created.
	 */
	void VerifyUserState();
	void OnResponseVerifyUserState(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/**
	 * Get path for read and write.
	 */
	FString GetPath() const noexcept;

	/**
	 * Write util data base's infos on the disk.
	 */
	void WriteDataBaseInfos() const;

	/**
	 * Read util data base's infos from the disk.
	 */
	void ReadDataBaseInfos();

	/**
	 * Create a new user into the data base.
	 * The user's id is his steam id.
	 * The guestToken returned by the data base is stored 
	 * to create sessions.
	 */
	void CreateNewUser();
	void OnResponseCreateNewUser(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/**
	 * Get the guestToken associated to the user.
	 * The user is already created.
	 * Function only used when the guestToken stored
	 * is corrupted.
	 */
	void GetUserGuestToken();
	void OnResponseGetUserGuestToken(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/**
	 * Create a sessionToken for the user.
	 * The sessionToken returned by the data base is stored
	 * to verify following data base connections.
	 */
	void CreateSession();
	void OnResponseCreateSession(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/**
	 * Refresh an oboslete sessionToken.
	 */
	void RefreshSession();
	void OnResponseRefreshSession(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/**
	 * Set up the user name using the steam nick name. 
	 */
	void SetUserName(const FString& Name);
	void OnResponseSetUserName(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/**
	 * Store the game save to the corresponding game id into the data base.
	 * 
	 * @param GameId, the game id
	 * @param GameSave, the json game save
	 */
	void StoreGameSave(const FString& GameId, const FString& GameSave);
	void OnResponseStoreGameSave(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/**
	 * Store a director save to the corresponding game id into the data base.
	 *
	 * @param GameId, the game id
	 * @param DirectorSave, the json director save
	 */
	void StoreDirectorSave(const FString& GameId, const FString& DirectorSave);
	void OnResponseStoreDirectorSave(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/**
	 * Store a player save to the corresponding game id into the data base.
	 * 
	 * @param GameId, the game id
	 * @param PlayerId, the player id
	 * @param PlayerSave, the json player save
	 */
	void StorePlayerSave(const FString& GameId, const FString& PlayerId, const FString& PlayerSave);
	void OnResponseStorePlayerSave(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/* LoadDirectorSave response */
	void OnResponseLoadDirectorSave(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/* LoadPlayerSaves response */
	void OnResponseLoadPlayerSaves(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/* DeleteSave response */
	void OnResponseDeleteSave(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/* GetAllSaves response */
	void OnResponseGetAllSaves(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/* GetPlayerScoreboard response */
	void OnResponseGetPlayerScoreboard(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/* IncreaseStatistics response */
	void OnResponseIncreaseStatistics(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/**
	 * Check the response's validity.
	 */
	bool ResponseCheck(FHttpResponsePtr Response, bool bWasSuccessful) const noexcept;

	/**
	 * Check the response's validity of a request using authentification.
	 */
	bool ResponseAuthCheck(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) noexcept;	
};
