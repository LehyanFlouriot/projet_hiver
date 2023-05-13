#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProjetHiver/Save/GameSave.h"
#include "LobbyWidget.generated.h"


class UFullscreenWidget;
class UScoreboard;

UCLASS()
class PROJETHIVER_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, Transient)
	UFullscreenWidget* LoadingScreen;

	UPROPERTY(BlueprintReadWrite, Transient)
	UScoreboard* Scoreboard;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetIntoLobby(const FString& Name, bool bIsCreator);

	UFUNCTION(BlueprintImplementableEvent)
	void RefreshLobby(const TArray<FLobbyInfo>& PlayerInfos);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowLaunchButton();

	UFUNCTION(BlueprintImplementableEvent)
	void BackToLobby();

	UFUNCTION(BlueprintImplementableEvent)
	void SetLoadedGame(const FString& GameName);

	UFUNCTION(BlueprintImplementableEvent)
	void ResetLoadedGame();

	UFUNCTION(BlueprintImplementableEvent)
	void RefreshGameSaves(const TArray<FGameSave>& GameSaves);

	FORCEINLINE UFullscreenWidget* GetLoadingScreen() const noexcept { return  LoadingScreen; }
	FORCEINLINE UScoreboard* GetScoreboard() const noexcept { return  Scoreboard; }
};
