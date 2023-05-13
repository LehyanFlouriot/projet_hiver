#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"

#include "ProjetHiver/LobbyCore/SessionInfo.h"
#include "ProjetHiver/Save/GameSave.h"
#include "ProjetHiver/Save/DirectorSave.h"

#include "EfhorisGameInstance.generated.h"

class UCompositeDataTable;
class UAkAudioEvent;
class ULevelSequence;
class UAkAudioEvent;

UCLASS()
class PROJETHIVER_API UEfhorisGameInstance : public UGameInstance
{
	GENERATED_BODY()

private:
	class IOnlineSubsystem* Subsystem;

	IOnlineIdentityPtr Identity;
	IOnlineSessionPtr SessionInterface;
	IOnlineFriendsPtr Friends;

	TSharedPtr<FOnlineSessionSearch> SessionSearchDedicated;
	TSharedPtr<FOnlineSessionSearch> SessionSearchListen;
	TArray<FSessionInfo> SessionInfos;

	//
	bool bIsLoggedIn = false;
	bool bHasCreatedTheSession = false;
	bool bHasJoinedTheSession = false;

	// 
	bool bHasBeenStarted = false;
	bool bShowLaunchButton = false;
	bool bHasLookedForListenServ = false;

	//
	FName SessionNameClient;

	//
	bool bShouldLoadGame = false;
	FGameSave GameSave;
	FDirectorSave DirectorSave;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ServerIP = "ec2-3-14-85-179.us-east-2.compute.amazonaws.com";

	// Items data tables
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCompositeDataTable* ArmorKitsDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCompositeDataTable* TrashItemsDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCompositeDataTable* WeaponKitsDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCompositeDataTable* GlovesDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCompositeDataTable* BeltsDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCompositeDataTable* RingsDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCompositeDataTable* NecklacesDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCompositeDataTable* EarringsDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCompositeDataTable* MerchantsDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCompositeDataTable* LootsDataTable;

	UPROPERTY(EditAnywhere)
	int NbConnections = 6;

	UPROPERTY(EditAnywhere)
	ULevelSequence* StartTileSequence;

	UPROPERTY(EditAnywhere)
	ULevelSequence* StartBossSequence;

	UPROPERTY(EditAnywhere)
	ULevelSequence* EndBossSequence;

	UPROPERTY(EditAnywhere)
	ULevelSequence* EndGameSequence;

public:

	UPROPERTY(EditAnywhere)
	UAkAudioEvent* HitSound;

	UPROPERTY(EditAnywhere)
	UAkAudioEvent* MenuMusic;

	UPROPERTY(EditAnywhere)
	UAkAudioEvent* SafeZoneMusic;

	UPROPERTY(EditAnywhere)
	UAkAudioEvent* AmbianceMusic;

	UPROPERTY(EditAnywhere)
	UAkAudioEvent* StartLevelMusic;

	UPROPERTY(EditAnywhere)
	UAkAudioEvent* Boss1Start;

	UPROPERTY(EditAnywhere)
	UAkAudioEvent* Boss1Defeated;

	UPROPERTY(EditAnywhere)
	UAkAudioEvent* Boss2Start;

	UPROPERTY(EditAnywhere)
	UAkAudioEvent* Boss2Defeated;

	UPROPERTY(EditAnywhere)
	UAkAudioEvent* Boss3Start;

	UPROPERTY(EditAnywhere)
	UAkAudioEvent* Boss3Defeated;



public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE ULevelSequence* GetStartTileSequence() {return StartTileSequence;}
	UFUNCTION(BlueprintCallable)
	FORCEINLINE ULevelSequence* GetStartBossSequence() {return StartBossSequence;}
	UFUNCTION(BlueprintCallable)
	FORCEINLINE ULevelSequence* GetEndBossSequence() {return EndBossSequence;}
	UFUNCTION(BlueprintCallable)
	FORCEINLINE ULevelSequence* GetEndGameSequence() {return EndGameSequence;}
	const FString& GetServerIP() const noexcept { return ServerIP; }

protected:
	virtual void Init() override;
	virtual void Shutdown() override;

	// Authentification et gestion des comptes

	UFUNCTION(BlueprintCallable)
	void Login();
	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

	UFUNCTION(BlueprintCallable)
	void GetAllFriends();
	void OnReadFriendListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);


	// Partie Creation et Lancement de session

	UFUNCTION(BlueprintCallable)
	void CreateServer(bool bPrivate = false);
	virtual void OnCreateSessionComplete(FName SessionName, bool Succeeded);

	UFUNCTION(BlueprintCallable)
	void ToggleReady();
	UFUNCTION(BlueprintCallable)
	void SetReady(bool b);

	UFUNCTION(BlueprintCallable)
	bool ArePlayersReady();

	UFUNCTION(BlueprintCallable)
	void LaunchSession(const FString& GameName);
	virtual void OnStartSessionComplete(FName SessionName, bool Succeeded);

	virtual void OnSessionParticipantsChange(FName SessionName, const FUniqueNetId& UniqueId, bool bJoined);

	virtual void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);
	virtual void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);


	// Partie Recherche de session

	UFUNCTION(BlueprintCallable)
	const TArray<FSessionInfo>& ListServers() const noexcept { return SessionInfos; }
	
	UFUNCTION(BlueprintCallable)
	void LookForServers();
	void LookForDedicatedServers();
	void LookForListenServers();
	virtual void OnFindSessionComplete(bool Succeeded);

	UFUNCTION(BlueprintCallable)
	void JoinServer(FName SessionName, FSessionInfo Session);
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);


	// Partie Sauvegarde

	UFUNCTION(BlueprintCallable)
	void GetAllGameSave() const;

	UFUNCTION(BlueprintCallable)
	void LoadGame(const FGameSave& GameSave_);

	UFUNCTION(BlueprintCallable)
	void UnLoadGame();

	UFUNCTION(BlueprintCallable)
	void DeleteSave(const FGameSave& GameSave_);

	/** Connect the player to the http server */
	void ConnectToHttp() const noexcept;

public:
	const UCompositeDataTable* GetArmorKitsDataTable() const;
	const UCompositeDataTable* GetTrashItemsDataTable() const;
	const UCompositeDataTable* GetWeaponKitsDataTable() const;
	const UCompositeDataTable* GetGlovesDataTable() const;
	const UCompositeDataTable* GetBeltsDataTable() const;
	const UCompositeDataTable* GetRingsDataTable() const;
	const UCompositeDataTable* GetNecklacesDataTable() const;
	const UCompositeDataTable* GetEarringsDataTable() const;
	const UCompositeDataTable* GetMerchantsDataTable() const;
	const UCompositeDataTable* GetLootsDataTable() const;

	bool HasCreatedTheSession() const noexcept { return bHasCreatedTheSession; }
	bool HasJoinedTheSession() const noexcept { return bHasJoinedTheSession; }
    
	void SetCreator() noexcept { bShowLaunchButton = bHasCreatedTheSession = true; }

	FString GetNickName() const noexcept {
		if (Identity.IsValid())
			return Identity->GetPlayerNickname(0);
		return { "Joueur" };
	}    
	FString GetPlayerId() const noexcept {
		if (Identity.IsValid())
			return Identity->GetUniquePlayerId(0)->ToString();
		return {};
	}

	bool ShouldLoadGame() const noexcept { return bShouldLoadGame; }
	void SetShouldLoadGame(const FGameSave& GameSave_) noexcept {
		bShouldLoadGame = true;
		GameSave = GameSave_;
	}
	void ResetShouldLoadGame() noexcept {
		GameSave = FGameSave();
		bShouldLoadGame = false;
	}
	FGameSave& GetGameSave() noexcept { return GameSave; }
	FDirectorSave& GetDirectorSave() noexcept { return DirectorSave; }
	void SetDirectorSave(FDirectorSave& DirectorSave_) noexcept { DirectorSave = DirectorSave_; }

	void SetGameName(const FString& GameName) noexcept { GameSave.GameName = GameName; }

	UFUNCTION(BlueprintCallable)
	void DestroyServer();

	/** Refresh lobby */
	void RefreshLobby();

	/** Set to the lobby the game saves from the data base. */
	void SetAllGameSaveFromDataBase(TArray<FGameSave> GameSaves) const;
};
