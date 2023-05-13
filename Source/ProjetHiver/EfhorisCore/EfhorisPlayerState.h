#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ProjetHiver/Save/PlayerSave.h"
#include "ProjetHiver/BDD/DataBaseSubsystem.h"
#include "ProjetHiver/BDD/UserScoreboard.h"
#include "EfhorisPlayerState.generated.h"

struct FUserInfos;
 
USTRUCT()
struct PROJETHIVER_API FInformations {
	GENERATED_BODY()

	UPROPERTY()
	FString NickName {};

	UPROPERTY()
	FString UserId {};

	UPROPERTY()
	bool bIsCreator = false;

	UPROPERTY()
	bool bIsLoggedIn = false;

	UPROPERTY()
	FUserInfos DataBaseInfos {};
};

UCLASS()
class PROJETHIVER_API AEfhorisPlayerState : public APlayerState
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() final override;

public:
	UFUNCTION(Client, Reliable)
	void Client_RefreshHealthPanel(const TArray<FPlayerInfo>& Descriptions);

	UFUNCTION(Client, Reliable)
	void Client_RefreshMapNicknames(const TArray<FPlayerInfo>& Descriptions);

	/**
	 * Get playerstate save.
	 *
	 * @return the object containing every relevant state to save PlayerState
	 */
	FPlayerSave GetPlayerStateSave() const noexcept;

	/**
	 * Set playerstate save.
	 *
	 * @param PlayerSave, the object containing every relevant state to save PlayerState
	 */
	void SetPlayerStateSave(const FPlayerSave& PlayerSave) noexcept;

	/**
	 * Write the save's files on creator of the game.
	 * 
	 * @param GameToSave, the path and the GameState's save object to write
	 * @param PlayerToSaves, array of path and PlayerState's save object to write
	 * @param bIsAsynch, is the save asynchronous
	 */
	UFUNCTION(Client, Reliable)
	void Client_WriteFilesOnCreator(const FFilesToSave& FilesToSave, bool bIsAsynch) const;

	UFUNCTION()
	void UploadStatistics() const;

	UFUNCTION()
	void IncreaseStatistic(EUserScoreboardStatistic ScoreboardStatistic, int Amount);

	UFUNCTION(Client, Reliable)
	void Client_GetInformations();

	UFUNCTION(Server, Reliable)
	void Server_SetInformations(const FInformations& Infos);

	UFUNCTION(BlueprintCallable)
	const FString& GetNickName() const noexcept { return SteamNickName; }

	UFUNCTION()
	const FString& GetSteamUserId() const noexcept { return SteamUserId; }

	UFUNCTION(BlueprintCallable)
	bool GetIsCreator() const noexcept { return bIsCreator; }

private:
	UPROPERTY(Replicated)
	FString SteamNickName;

	FString SteamUserId;

	bool bIsCreator;

	UPROPERTY(Replicated)
	FUserScoreboard Scoreboard;
};
