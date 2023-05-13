#include "LobbyLevel.h"

#include "ProjetHiver/LobbyCore/LobbyPlayerController.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameInstance.h"
#include "ProjetHiver/LobbyCore/LobbyPlayerState.h"

void ALobbyLevel::BeginPlay()
{
	Super::BeginPlay();

	if (const UWorld* World = GetWorld(); ensure(IsValid(World))) {
		if (const UEfhorisGameInstance* GameInstance = Cast<UEfhorisGameInstance>(World->GetGameInstance()); IsValid(GameInstance)) {
			if (ALobbyPlayerController* PlayerController = World->GetFirstPlayerController<ALobbyPlayerController>(); IsValid(PlayerController)) {
				MainWidget = CreateWidget<ULobbyWidget>(PlayerController, MainWidgetClass);
				MainWidget->AddToViewport();
				PlayerController->bShowMouseCursor = true;
				PlayerController->SetInputMode(FInputModeUIOnly{});

				if (GameInstance->HasCreatedTheSession()) {
					MainWidget->SetIntoLobby(GameInstance->GetNickName(), true);
				} else if (GameInstance->HasJoinedTheSession()) {
					MainWidget->SetIntoLobby(GameInstance->GetNickName(), false);
				}
			}
		}
	}
}
