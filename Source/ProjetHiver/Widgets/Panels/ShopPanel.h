#pragma once

#include "CoreMinimal.h"
#include "EfhorisPanel.h"
#include "ShopPanel.generated.h"

class AMerchant;

UCLASS(BlueprintType)
class PROJETHIVER_API UShopPanel : public UEfhorisPanel
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite)
	int SlotPerRow = 5;

	UPROPERTY(Transient, BlueprintReadWrite, EditAnywhere)
	AMerchant* Merchant;

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void RefreshPanel() override;

	UFUNCTION(BlueprintImplementableEvent)
	void RefreshShopPanel(AMerchant* _Merchant);
};
