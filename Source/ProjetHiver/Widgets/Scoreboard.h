#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Scoreboard.generated.h"

class UScrollBox;
enum class EUserScoreboardStatistic : uint8;

UCLASS()
class PROJETHIVER_API UScoreboard : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere,NoClear)
	TSubclassOf<UUserWidget> StatisticWidgetClass;

	UPROPERTY(BlueprintReadWrite, Transient)
	UScrollBox* ScrollBox;

    UPROPERTY(EditAnywhere)
	TMap<EUserScoreboardStatistic, FText> StatisticNames;

	UFUNCTION()
	void AddStatistic(const EUserScoreboardStatistic Type, const float Number);

public:
	UFUNCTION()
	void SetScoreboard(const FUserScoreboard& Scoreboard);

	UFUNCTION(BlueprintCallable)
	void UpdateScoreboard() const;
};
