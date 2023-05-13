#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreboardStatistic.generated.h"

class UTextBlock;

UCLASS()
class PROJETHIVER_API UScoreboardStatistic : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, Transient)
	UTextBlock* StatisticNameText;

	UPROPERTY(BlueprintReadWrite, Transient)
	UTextBlock* StatisticNumberText;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetStatisticName(const FText& Name) const;

	UFUNCTION(BlueprintImplementableEvent)
	void SetStatisticNumber(const float Number) const;
};