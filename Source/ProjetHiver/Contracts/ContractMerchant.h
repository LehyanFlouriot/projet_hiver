#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Interaction/InteractiveActor.h"
#include "ContractMerchant.generated.h"

UCLASS()
class PROJETHIVER_API AContractMerchant : public AActor, public IInteractiveActor
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
