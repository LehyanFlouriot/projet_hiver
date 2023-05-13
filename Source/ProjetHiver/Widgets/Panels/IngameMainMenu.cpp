#include "IngameMainMenu.h"

#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"

void UIngameMainMenu::Open() {
	if (!bIsMainMenuOpened) {
		if (IsValid(MainMenu)) {
			if (auto PC = Cast<AEfhorisPlayerController>(GetOwningPlayer()); IsValid(PC)) {
				PC->Client_ForceCloseAllMenus(true);
				PC->bIsMainMenuOpened = true;
			}
			MainMenu->OpenMenu();
			CurrentMenu = MainMenu;
		}
		bIsMainMenuOpened = true;
	}
}

void UIngameMainMenu::Close() {
	if (bIsMainMenuOpened) {
		if (IsValid(CurrentMenu)) {
			if (auto PC = Cast<AEfhorisPlayerController>(GetOwningPlayer()); IsValid(PC)) {
				PC->bIsMainMenuOpened = false;
			}
			CurrentMenu->CloseMenu();
			CurrentMenu = nullptr;
		}
		bIsMainMenuOpened = false;
	}
}
