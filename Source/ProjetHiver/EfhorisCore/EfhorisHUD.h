#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "EfhorisHUD.generated.h"

class UGameView;

UCLASS()
class PROJETHIVER_API AEfhorisHUD : public AHUD
{
	GENERATED_BODY()
protected:
	UPROPERTY(BlueprintReadWrite, Transient)
	UGameView* GameView;
public:
	UGameView* GetGameView() const;

	UFUNCTION()
	void HideGameView();

	UFUNCTION()
	void ShowGameView();

	UFUNCTION()
	void Win();
};
