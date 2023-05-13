#pragma once

#include "CoreMinimal.h"
#include "InteractiveActor.generated.h"

class APlayerCharacter;

UINTERFACE()
class PROJETHIVER_API UInteractiveActor : public UInterface
{
	GENERATED_BODY()
};

class IInteractiveActor {
	GENERATED_BODY()

public:
	virtual bool IsInteractible(const APlayerCharacter* Player) const = 0;
	virtual void StartInteract(APlayerCharacter* Player) = 0;
	virtual void StopInteract(APlayerCharacter* Player) = 0;
	virtual const FText GetInteractionText() const = 0;
};
