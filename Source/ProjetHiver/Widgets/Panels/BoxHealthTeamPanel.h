#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Widgets/Panels/EfhorisPanel.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "BoxHealthTeamPanel.generated.h"

UCLASS()
class PROJETHIVER_API UBoxHealthTeamPanel : public UEfhorisPanel
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent)
	void RefreshHealthTeamPanel(const TArray<FPlayerInfo>& Descriptions);
};
