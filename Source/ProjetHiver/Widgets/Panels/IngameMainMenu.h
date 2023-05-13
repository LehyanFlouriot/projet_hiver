#pragma once

#include "CoreMinimal.h"
#include "ToggleableMenu.h"
#include "IngameMainMenu.generated.h"

class UOptions;

UCLASS()
class PROJETHIVER_API UIngameMainMenu : public UUserWidget
{
	GENERATED_BODY()

	friend class UOptions;

protected:
	UPROPERTY(BlueprintReadWrite, Transient)
	UToggleableMenu* MainMenu;

	UPROPERTY(BlueprintReadWrite, Transient)
	UToggleableMenu* OptionsMenu;

	UPROPERTY(BlueprintReadWrite, Transient)
	UToggleableMenu* GameOptions;

	UPROPERTY(BlueprintReadWrite, Transient)
	UToggleableMenu* VideoOptions;

	UPROPERTY(BlueprintReadWrite, Transient)
	UToggleableMenu* AudioOptions;

	UPROPERTY(BlueprintReadWrite,Transient)
	bool bIsMainMenuOpened = false;

	UPROPERTY(BlueprintReadWrite,Transient)
	UToggleableMenu* CurrentMenu;

public:
	UFUNCTION(BlueprintCallable)
	void Open();

	UFUNCTION(BlueprintCallable)
	void Close();

	UFUNCTION()
	UToggleableMenu* GetGameOptions() noexcept { return GameOptions; }
};
