#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "LobbyWidget.h"
#include "LobbyLevel.generated.h"

/**
 * 
 */
UCLASS()
class PROJETHIVER_API ALobbyLevel : public ALevelScriptActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	ULobbyWidget* GetMainWidget() noexcept { return MainWidget; }
	const ULobbyWidget* GetMainWidget() const noexcept { return MainWidget; }

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<ULobbyWidget> MainWidgetClass;
	
	UPROPERTY(Transient)
	ULobbyWidget* MainWidget;
};
