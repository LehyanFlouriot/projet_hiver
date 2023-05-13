#pragma once

#include "CoreMinimal.h"
#include "EfhorisPanel.h"
#include "SmithPanel.generated.h"

UCLASS()
class PROJETHIVER_API USmithPanel : public UEfhorisPanel
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void RefreshPanel() override;
};
