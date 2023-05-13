#include "ProjetHiver/Widgets/Panels/Options.h"
#include "ProjetHiver/Widgets/Panels/IngameMainMenu.h"


void UOptions::OpenChildMenu(UToggleableMenu* Child) {
	if (IsValid(Child) && !bIsInCooldown) {
		Child->OpenMenu();
		CloseMenu(false,true);
		if (IsValid(MainMenu)) {
			MainMenu->CurrentMenu = Child;
		}
	}
}

void UOptions::CloseSelfAndOpenParent() {
	if (IsValid(ParentMenu) && !bIsInCooldown) {
		ParentMenu->OpenMenu();
		CloseMenu(false, true);
		if (IsValid(MainMenu)) {
			MainMenu->CurrentMenu = ParentMenu;
		}
	}
}
