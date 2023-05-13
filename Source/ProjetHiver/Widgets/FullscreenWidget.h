#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Widgets/Panels/ToggleableMenu.h"
#include "FullscreenWidget.generated.h"

UCLASS()
class PROJETHIVER_API UFullscreenWidget : public UToggleableMenu
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Show();

	UFUNCTION(BlueprintCallable)
	void Hide();
};

inline void UFullscreenWidget::Show() {
	OpenMenu();
	if (auto Player = GetOwningPlayer(); IsValid(Player)) {
		Player->SetShowMouseCursor(true);
		Player->SetInputMode(FInputModeUIOnly{});
	}
}

inline void UFullscreenWidget::Hide() {
	CloseMenu();
}
