#pragma once

#include "CoreMinimal.h"
#include "ToggleableMenu.h"
#include "EfhorisPanel.generated.h"

UCLASS(Abstract)
class PROJETHIVER_API UEfhorisPanel : public UToggleableMenu
{
	GENERATED_BODY()

public:
	virtual void RefreshPanel() {}
};
