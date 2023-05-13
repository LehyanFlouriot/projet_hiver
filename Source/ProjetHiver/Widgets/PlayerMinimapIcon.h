#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerMinimapIcon.generated.h"

UCLASS()
class PROJETHIVER_API UPlayerMinimapIcon : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetNickname(const FString& Name);
};
