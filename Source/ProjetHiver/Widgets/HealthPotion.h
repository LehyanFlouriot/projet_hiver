#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthPotion.generated.h"

/**
 * 
 */
UCLASS()
class PROJETHIVER_API UHealthPotion : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void RefreshHealthPotion();
};
