#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Widgets/Panels/ToggleableMenu.h"
#include "Options.generated.h"

class UIngameMainMenu;

UCLASS()
class PROJETHIVER_API UOptions : public UToggleableMenu
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite,Transient)
	UIngameMainMenu* MainMenu;

	UPROPERTY(BlueprintReadWrite, Transient)
	UToggleableMenu* ParentMenu;

public:
	UFUNCTION(BlueprintCallable)
	void OpenChildMenu(UToggleableMenu* Child);

	UFUNCTION(BlueprintCallable)
	void CloseSelfAndOpenParent();

	UFUNCTION(BlueprintImplementableEvent)
	void OnCreatorSeted();
};
