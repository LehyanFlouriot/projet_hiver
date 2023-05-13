#pragma once

#include "CoreMinimal.h"
#include "Contract.h"
#include "AccumulationContract.generated.h"

enum class EAccumulationContractType : uint8;

UCLASS()
class PROJETHIVER_API AAccumulationContract final : public AContract
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Replicated, ReplicatedUsing=OnRep_CountRep)
	int Count;

	UPROPERTY(EditAnywhere)
	EAccumulationContractType Type;

public:
	int GetCount() const noexcept;
	void SetCount(const int NewCount) noexcept;
	void IncreaseCount() noexcept;

	EAccumulationContractType GetType() const noexcept;

	UFUNCTION()
	void OnRep_CountRep() const;
};
