#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjetHiver/Interaction/InteractiveActor.h"
#include "SmithMerchant.generated.h"

struct FItemInfo;

UCLASS()
class PROJETHIVER_API ASmithMerchant final : public AActor, public IInteractiveActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	FText InteractionText;

	UPROPERTY(EditAnywhere)
	FText BringBackHammerText;

public:
	virtual bool IsInteractible(const APlayerCharacter* Player) const override;
	virtual void StartInteract(APlayerCharacter* Player) override;
	virtual void StopInteract(APlayerCharacter* Player) override;
	virtual const FText GetInteractionText() const override;
	
};
