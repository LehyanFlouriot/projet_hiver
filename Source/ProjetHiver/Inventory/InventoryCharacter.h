#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ProjetHiver/Interaction/InteractiveActor.h"
#include "InventoryCharacter.generated.h"

class APlayerCharacter;

UCLASS(Abstract)
class PROJETHIVER_API AInventoryCharacter : public ACharacter, public IInteractiveActor
{
	GENERATED_BODY()

public:
	AInventoryCharacter(const FObjectInitializer& ObjectInitializer);

	virtual bool IsInteractible(const APlayerCharacter* Player) const override;
	virtual void StartInteract(APlayerCharacter* Player) override;
	virtual void StopInteract(APlayerCharacter* Player) override;
	virtual const FText GetInteractionText() const override;

	UFUNCTION(BlueprintCallable)
	APlayerCharacter* GetTarget() const noexcept { return Target; }

	UFUNCTION(BlueprintCallable)
	void SetTarget(APlayerCharacter* Target_);

	UFUNCTION(BlueprintCallable)
	void RemoveTarget();

	UFUNCTION(BlueprintCallable)
	bool HasTarget() const noexcept { return Target != nullptr; }

	void SetTargetId(const FString& Id) noexcept { TargetId = Id; }

	const FString& GetTargetId() const noexcept { return TargetId; }

	bool HasTargetId() const noexcept { return !TargetId.IsEmpty(); }

	UFUNCTION()
	void ResetRebuild();

protected:
	UPROPERTY(Transient, Replicated, BlueprintReadOnly)
	int NbPlayerInteracting = 0;

	UPROPERTY(Transient, Replicated)
	APlayerCharacter* Target = nullptr;

	FString TargetId;

	UPROPERTY(Transient, Replicated, BlueprintReadOnly)
	bool bIsOpened;

	UPROPERTY(EditAnywhere)
	FText InteractionText;

	const float InteractRangeOffset = 50.f;
	float DistanceToTarget() const;

	void TpChestNearTarget();
};
