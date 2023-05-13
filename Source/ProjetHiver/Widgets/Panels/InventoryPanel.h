#pragma once

#include "CoreMinimal.h"
#include "EfhorisPanel.h"
#include "InventoryPanel.generated.h"

UCLASS(BlueprintType)
class PROJETHIVER_API UInventoryPanel : public UEfhorisPanel
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite)
	int SlotPerRow = 5;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void RefreshPanel() override;
};
