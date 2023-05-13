#include "LobbyPlayerController.h"

#include "GameFramework/GameStateBase.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameInstance.h"
#include "ProjetHiver/Widgets/FullscreenWidget.h"
#include "ProjetHiver/Widgets/Scoreboard.h"
#include "ProjetHiver/LobbyCore/LobbyGameMode.h"
#include "ProjetHiver/LobbyCore/LobbyPlayerState.h"
#include "ProjetHiver/Widgets/Lobby/LobbyLevel.h"

void ALobbyPlayerController::Authority_SetCreator() noexcept
{
	bIsCreator = true;

	Client_SetCreator();
}

void ALobbyPlayerController::SetScoreboard(const FUserScoreboard& Scoreboard) const {
	if (ALobbyLevel* LobbyLevelScript = Cast<ALobbyLevel>(GetLevel()->GetLevelScriptActor()); IsValid(LobbyLevelScript))
		if (const ULobbyWidget* MenuWidget = LobbyLevelScript->GetMainWidget(); IsValid(MenuWidget))
			if (UScoreboard* ScoreboardWidget = MenuWidget->GetScoreboard(); IsValid(ScoreboardWidget))
				ScoreboardWidget->SetScoreboard(Scoreboard);
}

void ALobbyPlayerController::Server_EveryoneOpenLoadingScreen_Implementation() const {
	for (auto State : GetWorld()->GetGameState()->PlayerArray) {
		if (State) {
			if (ALobbyPlayerController* PlayerController = Cast<ALobbyPlayerController>(State->GetPlayerController()); IsValid(PlayerController)) {
				PlayerController->Client_OpenLoadingScreen();
			}
		}
	}
}

void ALobbyPlayerController::Client_CloseLoadingScreen_Implementation() const {
	if (ALobbyLevel* LobbyLevelScript = Cast<ALobbyLevel>(GetLevel()->GetLevelScriptActor()); IsValid(LobbyLevelScript))
		if (ULobbyWidget* MenuWidget = LobbyLevelScript->GetMainWidget(); IsValid(MenuWidget)) {
			if (UFullscreenWidget* LoadingScreen = MenuWidget->GetLoadingScreen(); IsValid(LoadingScreen))
			{
				LoadingScreen->ForceCloseMenu(true);
			}
		}
}

void ALobbyPlayerController::Client_OpenLoadingScreen_Implementation() const {if (ALobbyLevel* LobbyLevelScript = Cast<ALobbyLevel>(GetLevel()->GetLevelScriptActor()); IsValid(LobbyLevelScript))
		if (ULobbyWidget* MenuWidget = LobbyLevelScript->GetMainWidget(); IsValid(MenuWidget)) {
			if (UFullscreenWidget* LoadingScreen = MenuWidget->GetLoadingScreen(); IsValid(LoadingScreen))
			{
				LoadingScreen->Show();
			}
		}
}

void ALobbyPlayerController::Client_SetCreator_Implementation()
{
	if (UEfhorisGameInstance* GameInstance = Cast<UEfhorisGameInstance>(GetGameInstance()); IsValid(GameInstance)) {
		UE_LOG(LogTemp, Warning, TEXT("Become the creator of the session"));
		
		GameInstance->SetCreator();
	}
}

void ALobbyPlayerController::Server_ToggleReady_Implementation()
{
	if (ALobbyPlayerState* PlayerState_ = GetPlayerState<ALobbyPlayerState>(); IsValid(PlayerState_)) {
		PlayerState_->ToggleIsReady();
	}
}

void ALobbyPlayerController::Server_LaunchSession_Implementation(const FString& GameName)
{
	if (UEfhorisGameInstance* GameInstance = Cast<UEfhorisGameInstance>(GetGameInstance()); IsValid(GameInstance)) {
		GameInstance->SetGameName(GameName);

		GetWorld()->ServerTravel("/Game/MapGeneration/Maps/Overview?listen");
	}
		
}

void ALobbyPlayerController::Server_SetGameLoaded_Implementation(const FGameSave& GameSave_)
{
	if(const UWorld* World = GetWorld(); IsValid(World))
		if (ALobbyGameMode* GameMode = World->GetAuthGameMode<ALobbyGameMode>(); IsValid(GameMode))
			GameMode->SetLoadedGame(GameSave_);
}

void ALobbyPlayerController::Server_ResetGameLoaded_Implementation()
{
	if (const UWorld * World = GetWorld(); IsValid(World))
		if (ALobbyGameMode* GameMode = World->GetAuthGameMode<ALobbyGameMode>(); IsValid(GameMode))
			GameMode->ResetLoadedGame();
}

void ALobbyPlayerController::Client_SetGameLoaded_Implementation(const FString& GameName) const
{
	if (ALobbyLevel* LobbyLevelScript = Cast<ALobbyLevel>(GetLevel()->GetLevelScriptActor()); IsValid(LobbyLevelScript))
		if (ULobbyWidget* MenuWidget = LobbyLevelScript->GetMainWidget(); IsValid(MenuWidget)) {
			MenuWidget->SetLoadedGame(GameName);
		}
}

void ALobbyPlayerController::Client_ResetGameLoaded_Implementation() const
{
	if (ALobbyLevel* LobbyLevelScript = Cast<ALobbyLevel>(GetLevel()->GetLevelScriptActor()); IsValid(LobbyLevelScript))
		if (ULobbyWidget* MenuWidget = LobbyLevelScript->GetMainWidget(); IsValid(MenuWidget)) {		
			MenuWidget->ResetLoadedGame();
		}
}

void ALobbyPlayerController::Server_SetDirectorSave_Implementation(const FDirectorSave& DirectorSave) const
{
	if (const UWorld* World = GetWorld(); IsValid(World))
		if (ALobbyGameMode* GameMode = World->GetAuthGameMode<ALobbyGameMode>(); IsValid(GameMode))
			GameMode->SetDirectorSave(DirectorSave);
}

void ALobbyPlayerController::Server_SetPlayerSaves_Implementation(const FPlayerSaves& PlayerSaves) const
{
	if (const UWorld* World = GetWorld(); IsValid(World))
		if (ALobbyGameMode* GameMode = World->GetAuthGameMode<ALobbyGameMode>(); IsValid(GameMode))
			GameMode->SetPlayerSaves(PlayerSaves);
}

void ALobbyPlayerController::Server_CancelLoading_Implementation() const
{
	if (const UWorld* World = GetWorld(); IsValid(World))
		if (ALobbyGameMode* GameMode = World->GetAuthGameMode<ALobbyGameMode>(); IsValid(GameMode))
			GameMode->CancelLoading();
}
