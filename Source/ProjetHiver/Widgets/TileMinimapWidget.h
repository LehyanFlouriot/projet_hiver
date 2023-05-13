#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TileMinimapWidget.generated.h"

UCLASS()
class PROJETHIVER_API UTileMinimapWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient)
	UTexture2D* TileTexture;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetTileTexture(UTexture2D* Texture);
};
