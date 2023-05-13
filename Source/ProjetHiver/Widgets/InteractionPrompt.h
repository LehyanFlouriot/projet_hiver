#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionPrompt.generated.h"

class UTextBlock;

UCLASS(Abstract)
class PROJETHIVER_API UInteractionPrompt final : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetText(const FText& Text) const;

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* InteractionText;
};
