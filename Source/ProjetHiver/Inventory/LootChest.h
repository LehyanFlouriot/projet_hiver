#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjetHiver/Interaction/InteractiveActor.h"
#include "LootChest.generated.h"

class ULootComponent;

UCLASS()
class PROJETHIVER_API ALootChest final : public AActor, public IInteractiveActor 
{
	GENERATED_BODY()

public:
	ALootChest(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditAnywhere)
	ULootComponent* LootComponent;

	UPROPERTY(Transient,Replicated)
	bool bIsInteractible = true;

	UPROPERTY(Transient, Replicated, BlueprintReadOnly)
	bool bIsOpened = false;

	UPROPERTY(EditAnywhere)
	FText InteractionText;

	UPROPERTY(EditAnywhere)
	USceneComponent* HealSeedLocation;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual bool IsInteractible(const APlayerCharacter* Player) const override;
	virtual void StartInteract(APlayerCharacter* Player) override;
	virtual void StopInteract(APlayerCharacter* Player) override;
	virtual const FText GetInteractionText() const override;
	USceneComponent* GetSeedLocation() const;
	bool IsOpened() const noexcept { return bIsOpened; }
	void Open();
};
