#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Contract.generated.h"

enum class EContractStatus : uint8;

UCLASS(Abstract, Blueprintable)
class PROJETHIVER_API AContract : public AInfo
{
	GENERATED_BODY()

public:
	AContract();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Summary;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine=true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText HUDSummary;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Cost;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Reward;

	UPROPERTY(Transient, BlueprintReadWrite, Replicated, ReplicatedUsing=OnRep_StatusRep)
	EContractStatus Status;

public:
	FText GetTitle() const noexcept;
	FText GetSummary() const noexcept;
	FText GetDescription() const noexcept;
	FText GetHUDSummary() const noexcept;
	int GetCost() const noexcept;
	int GetReward() const noexcept;

	EContractStatus GetStatus() const noexcept;
	void SetStatus(const EContractStatus NewStatus) noexcept;

	UFUNCTION()
	void OnRep_StatusRep() const;

protected:
	void RefreshContractPanels(const bool bRefreshOpenedPanels = false) const;
};
