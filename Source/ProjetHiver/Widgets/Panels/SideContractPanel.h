#pragma once

#include "CoreMinimal.h"
#include "EfhorisPanel.h"
#include "SideContractPanel.generated.h"

UCLASS(BlueprintType)
class PROJETHIVER_API USideContractPanel : public UEfhorisPanel
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void RefreshPanel() override;

	UFUNCTION(BlueprintImplementableEvent)
	void RefreshSideContractPanel();
};
