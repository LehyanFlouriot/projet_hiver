#pragma once

#include "CoreMinimal.h"
#include "EfhorisPanel.h"
#include "MedicPanel.generated.h"

class UBoxHealthTeamPanel;

UCLASS()
class PROJETHIVER_API UMedicPanel : public UEfhorisPanel
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, Transient)
	UBoxHealthTeamPanel* MedicBoxHealthTeamPanel;

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void RefreshPanel() override;

	UFUNCTION(BlueprintCallable)
	void RefreshHealthTeamPanel(const TArray<FPlayerInfo>& Descriptions);
};
