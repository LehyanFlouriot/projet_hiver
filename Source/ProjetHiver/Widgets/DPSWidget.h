#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DPSWidget.generated.h"

class ADummy;

UCLASS()
class PROJETHIVER_API UDPSWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient, BlueprintReadOnly)
	float DPS;

public:
	void SetDPS(const float Dps) { DPS = Dps; }
};
