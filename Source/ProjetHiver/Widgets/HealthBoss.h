#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Widgets/Panels/EfhorisPanel.h"
#include "HealthBoss.generated.h"

UCLASS()
class PROJETHIVER_API UHealthBoss : public UEfhorisPanel
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void StartBoss(const ADragonBoss* Boss);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateBoss();

	UFUNCTION(BlueprintImplementableEvent)
	void EndBoss();
};
