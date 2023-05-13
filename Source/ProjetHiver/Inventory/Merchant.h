#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ProjetHiver/Interaction/InteractiveActor.h"
#include "Merchant.generated.h"

struct FItemInfo;

UCLASS()
class PROJETHIVER_API AMerchant : public ACharacter, public IInteractiveActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	FName RowName;

	UPROPERTY(Transient)
	int Level;

	UPROPERTY(Transient, Replicated, BlueprintReadWrite, EditAnywhere)
	TArray<FItemInfo> Stock;

	UPROPERTY(Transient)
	bool bIsInfinite;

	UPROPERTY(EditAnywhere)
	FText InteractionText;

	virtual void BeginPlay() override;

public:
	virtual bool IsInteractible(const APlayerCharacter* Player) const override;
	virtual void StartInteract(APlayerCharacter* Player) override;
	virtual void StopInteract(APlayerCharacter* Player) override;
	virtual const FText GetInteractionText() const override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	bool IsInfinite() const noexcept;

	UFUNCTION()
	void RemoveItemFromStock(FItemInfo ItemInfo);

protected:
	void GenerateStock();
};
