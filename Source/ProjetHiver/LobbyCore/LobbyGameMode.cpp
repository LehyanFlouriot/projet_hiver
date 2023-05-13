#include "LobbyGameMode.h"

#include "ProjetHiver/EfhorisCore/EfhorisGameInstance.h"
#include "ProjetHiver/LobbyCore/LobbyPlayerController.h"
#include "ProjetHiver/LobbyCore/LobbyGameStateBase.h"
#include "ProjetHiver/LobbyCore/LobbyPlayerState.h"
#include "ProjetHiver/Save/SaveSubsystem.h"
#include "SocketSubsystem.h"

void ALobbyGameMode::OnPostLogin(AController* NewPlayer)
{
	if (ALobbyPlayerController* NewPlayerController = Cast<ALobbyPlayerController>(NewPlayer); IsValid(NewPlayerController))
		if (HasActorBegunPlay())
			if (ALobbyGameStateBase* LobbyGameState = GetGameState<ALobbyGameStateBase>(); IsValid(LobbyGameState))
				if (LobbyGameState->PlayerArray.Num() == 1 || LobbyGameState->PlayerArray.Num() == 0) {
					NewPlayerController->Authority_SetCreator();
				} else {
					if(LobbyGameState->IsLoadedGame())
						NewPlayerController->Client_SetGameLoaded(LobbyGameState->GetGameName());
				}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	if (ALobbyPlayerController* ExitingController = Cast<ALobbyPlayerController>(Exiting); IsValid(ExitingController))
		if (ExitingController->IsCreator())
			if (ALobbyGameStateBase* LobbyGameState = GetGameState<ALobbyGameStateBase>(); IsValid(LobbyGameState))
				if (LobbyGameState->PlayerArray.Num() > 1)
					if (ALobbyPlayerState* NewCreatorState = Cast<ALobbyPlayerState>(LobbyGameState->PlayerArray[1]); IsValid(NewCreatorState))
						if (ALobbyPlayerController* NewCreator = Cast<ALobbyPlayerController>(NewCreatorState->GetPlayerController()); IsValid(NewCreator)) {
							NewCreator->Authority_SetCreator();
							if (LobbyGameState->IsLoadedGame())
								ResetLoadedGame();
						}
}

void ALobbyGameMode::SetLoadedGame(const FGameSave& GameSave_)
{
	if (ALobbyGameStateBase* LobbyGameState = GetGameState<ALobbyGameStateBase>(); LobbyGameState) {
		LobbyGameState->SetGameSave(GameSave_);

		if (UEfhorisGameInstance* GameInstance = Cast<UEfhorisGameInstance>(GetGameInstance()); IsValid(GameInstance))
			GameInstance->SetShouldLoadGame(GameSave_);
	}	
}

void ALobbyGameMode::SetDirectorSave(FDirectorSave DirectorSave)
{
	bHasLoadDirectorSave = true;
	if (UEfhorisGameInstance* GameInstance = Cast<UEfhorisGameInstance>(GetGameInstance()); IsValid(GameInstance)) {
		GameInstance->SetDirectorSave(DirectorSave);
	}

	if (bHasLoadDirectorSave && bHasLoadPlayerSaves)
		CloseLoadingMenu();
}

void ALobbyGameMode::SetPlayerSaves(const FPlayerSaves& PlayerSaves)
{
	bHasLoadPlayerSaves = true;

	USaveSubsystem* Save = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	Save->StorePlayerStateSaves(PlayerSaves);

	if (bHasLoadDirectorSave && bHasLoadPlayerSaves)
		CloseLoadingMenu();
}

void ALobbyGameMode::ResetLoadedGame()
{
	if (ALobbyGameStateBase* LobbyGameState = GetGameState<ALobbyGameStateBase>(); LobbyGameState) {
		LobbyGameState->ResetGameSave();

		if (UEfhorisGameInstance* GameInstance = Cast<UEfhorisGameInstance>(GetGameInstance()); IsValid(GameInstance)) {
			GameInstance->ResetShouldLoadGame();
		}

		bHasLoadPlayerSaves = bHasLoadDirectorSave = false;

		USaveSubsystem* Save = GetGameInstance()->GetSubsystem<USaveSubsystem>();
		Save->ResetSaves();

		for (const auto& Player : LobbyGameState->PlayerArray) {
			if (const ALobbyPlayerController* Controller = Cast<ALobbyPlayerController>(Player->GetPlayerController()); IsValid(Controller)) {
				Controller->Client_ResetGameLoaded();
			}
		}
	}
}

void ALobbyGameMode::CancelLoading()
{
	ResetLoadedGame();

	if (ALobbyGameStateBase* LobbyGameState = GetGameState<ALobbyGameStateBase>(); LobbyGameState) {
		for (const auto& Player : LobbyGameState->PlayerArray) {
			if (const ALobbyPlayerController* Controller = Cast<ALobbyPlayerController>(Player->GetPlayerController()); IsValid(Controller)) {
				if (Controller->IsCreator()) Controller->Client_CloseLoadingScreen();
			}
		}
	}
}

void ALobbyGameMode::CloseLoadingMenu() const noexcept
{
	UE_LOG(LogTemp, Warning, TEXT("Close Loading Menu"));

	if (ALobbyGameStateBase* LobbyGameState = GetGameState<ALobbyGameStateBase>(); LobbyGameState) {
		for (const auto& Player : LobbyGameState->PlayerArray) {
			if (const ALobbyPlayerController* Controller = Cast<ALobbyPlayerController>(Player->GetPlayerController()); IsValid(Controller)) {
				if(Controller->IsCreator()) Controller->Client_CloseLoadingScreen();
				Controller->Client_SetGameLoaded(LobbyGameState->GetGameName());
			}
		}
	}
}
