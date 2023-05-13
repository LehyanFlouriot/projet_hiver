#pragma once

#include "CoreMinimal.h"
#include "Contract.h"
#include "FailableContract.generated.h"

enum class EFailableContractType : uint8;

UCLASS()
class PROJETHIVER_API AFailableContract final : public AContract
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Limit;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, ReplicatedUsing = OnRep_CurrentValueRep)
	int CurrentValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EFailableContractType Type;

public:
	int GetLimit() const noexcept;

	int GetCurrentValue() const noexcept;
	void SetCurrentValue(const int NewValue) noexcept;
	void IncreaseCurrentValue() noexcept;

	EFailableContractType GetType() const noexcept;

	UFUNCTION()
	void OnRep_CurrentValueRep() const;
};
