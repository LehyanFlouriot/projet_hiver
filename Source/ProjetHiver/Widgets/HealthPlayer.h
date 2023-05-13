#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Widgets/Panels/EfhorisPanel.h"
#include "HealthPlayer.generated.h"

UCLASS()
class PROJETHIVER_API UHealthPlayer : public UEfhorisPanel
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdatePlayer(const APlayerCharacter* Player);
};
