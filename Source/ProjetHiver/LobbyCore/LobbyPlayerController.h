#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

struct FGameSave;
struct FDirectorSave;
struct FPlayerSaves;

/**
 * 
 */
UCLASS()
class PROJETHIVER_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

	bool bIsCreator = false;

public:
	bool IsCreator() const noexcept { return bIsCreator; }

	void Authority_SetCreator() noexcept;

private:
	UFUNCTION(Client, Reliable)
	void Client_SetCreator();

public:
	UFUNCTION(Server, Reliable)
	void Server_ToggleReady();

	UFUNCTION(Server, Reliable)
	void Server_LaunchSession(const FString& GameName);

	UFUNCTION(Server, Reliable)
	void Server_SetGameLoaded(const FGameSave& GameSave);

	UFUNCTION(Server, Reliable)
	void Server_ResetGameLoaded();

	UFUNCTION(Client, Reliable)
	void Client_SetGameLoaded(const FString& GameName) const;

	UFUNCTION(Client, Reliable)
	void Client_ResetGameLoaded() const;

	UFUNCTION(Client, Reliable)
	void Client_OpenLoadingScreen() const;

	UFUNCTION(Client, Reliable)
	void Client_CloseLoadingScreen() const;

	UFUNCTION(Server, Reliable)
	void Server_EveryoneOpenLoadingScreen() const;

	UFUNCTION(Server, Reliable)
	void Server_SetDirectorSave(const FDirectorSave& DirectorSave) const;

	UFUNCTION(Server, Reliable)
	void Server_SetPlayerSaves(const FPlayerSaves& PlayerSaves) const;

	UFUNCTION(Server, Reliable)
	void Server_CancelLoading() const;

	UFUNCTION()
	void SetScoreboard(const FUserScoreboard& Scoreboard) const;
};
