#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

struct FGameSave;
struct FDirectorSave;
struct FPlayerSaves;

/**
 * 
 */
UCLASS()
class PROJETHIVER_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

	bool bHasLoadDirectorSave = false;
	bool bHasLoadPlayerSaves = false;

protected:
	virtual void OnPostLogin(AController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

public:
	void SetLoadedGame(const FGameSave& GameSave_);

	void SetDirectorSave(FDirectorSave DirectorSave);

	void SetPlayerSaves(const FPlayerSaves& PlayerSaves);

	void ResetLoadedGame();

	void CancelLoading();

private:
	void CloseLoadingMenu() const noexcept;
};
