#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ProjetHiver/Interaction/InteractiveActor.h"
#include "Kevin.generated.h"

class APlayerCharacter;

UCLASS(Abstract)
class PROJETHIVER_API AKevin : public ACharacter, public IInteractiveActor
{
	GENERATED_BODY()

public:
	AKevin(const FObjectInitializer& ObjectInitializer);

	virtual bool IsInteractible(const APlayerCharacter* Player) const override;
	virtual void StartInteract(APlayerCharacter* Player) override;
	virtual void StopInteract(APlayerCharacter* Player) override;
	virtual const FText GetInteractionText() const override;

	UFUNCTION(BlueprintCallable)
	APlayerCharacter* GetTarget() const noexcept { return Target; }

	UFUNCTION(BlueprintCallable)
	void SetTarget(APlayerCharacter* Target_) noexcept;

	UFUNCTION(BlueprintCallable)
	bool HasTarget() const noexcept;

	void SetTargetId(const FString& Id) noexcept { TargetId = Id; }

	const FString& GetTargetId() const noexcept { return TargetId; }

	bool HasTargetId() const noexcept { return !TargetId.IsEmpty(); }

	UFUNCTION(BlueprintCallable)
	void ReachGoal();

protected:
	UPROPERTY(Transient, Replicated)
	APlayerCharacter* Target = nullptr;

	UPROPERTY()
	FString TargetId;

	UPROPERTY(EditAnywhere)
	FText InteractionText;

	UPROPERTY(EditAnywhere)
	FText InteractionTextChangeOwner;
};
