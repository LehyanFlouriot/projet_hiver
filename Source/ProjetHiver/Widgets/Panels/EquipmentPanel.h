#pragma once

#include "CoreMinimal.h"
#include "EfhorisPanel.h"
#include "EquipmentPanel.generated.h"

UCLASS(BlueprintType)
class PROJETHIVER_API UEquipmentPanel : public UEfhorisPanel
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void RefreshPanel() override;
};
