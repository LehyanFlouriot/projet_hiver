#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Interaction/InteractiveActor.h"
#include "ProjetHiver/Items/ReplicatedStaticMeshActor.h"
#include "QuestItem.generated.h"

UCLASS(Abstract)
class PROJETHIVER_API AQuestItem final : public AReplicatedStaticMeshActor, public IInteractiveActor
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
