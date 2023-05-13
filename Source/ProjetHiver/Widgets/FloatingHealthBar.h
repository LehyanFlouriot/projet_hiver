#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FloatingHealthBar.generated.h"

class UProgressBar;

UCLASS(Abstract)
class PROJETHIVER_API UFloatingHealthBar final : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHealthPercentage(float NewHealth) const;

	UFUNCTION(BlueprintImplementableEvent)
	void SetColor(const FColor& Color) const;

protected:
	UPROPERTY(meta = (BindWidget),BlueprintReadWrite)
	UProgressBar* HealthBar;
};
