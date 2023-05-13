#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ProjetHiver/Save/GameSave.h"
#include "LobbyGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJETHIVER_API ALobbyGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FGameSave GameSave;
	
public:
	bool IsLoadedGame() const noexcept { return !(GameSave == FGameSave{}); }

	FString GetGameId() const noexcept { return GameSave.GameId.ToString(); }

	const FString& GetGameName() const noexcept { return GameSave.GameName; }

	void SetGameSave(const FGameSave& GameSave_) noexcept { GameSave = GameSave_; }

	void ResetGameSave() noexcept { GameSave = {}; }
};
