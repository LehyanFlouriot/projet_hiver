#pragma once

#include "CoreMinimal.h"
#include "Contract.h"
#include "ExplorationContract.generated.h"

enum class EExplorationContractType : uint8;

UCLASS()
class PROJETHIVER_API AExplorationContract final : public AContract
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EExplorationContractType Type;

public:
	EExplorationContractType GetType() const noexcept;
};