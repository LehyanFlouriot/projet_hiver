#pragma once

#include "CoreMinimal.h"
#include "EfhorisPanel.h"
#include "ContractPanel.generated.h"

class AContract;

UCLASS(BlueprintType)
class PROJETHIVER_API UContractPanel : public UEfhorisPanel
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient, BlueprintReadWrite, EditAnywhere)
	AContract* Contract;

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void RefreshPanel() override;

	UFUNCTION(BlueprintImplementableEvent)
	void RefreshContractPanel(AContract* _Contract);
};
