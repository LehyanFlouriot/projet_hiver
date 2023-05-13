#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjetHiver/Interaction/InteractiveActor.h"
#include "MedicMerchant.generated.h"

UCLASS()
class PROJETHIVER_API AMedicMerchant final : public AActor, public IInteractiveActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	FText InteractionText;

public:
	virtual bool IsInteractible(const APlayerCharacter* Player) const override;
	virtual void StartInteract(APlayerCharacter* Player) override;
	virtual void StopInteract(APlayerCharacter* Player) override;
	virtual const FText GetInteractionText() const override;

};
