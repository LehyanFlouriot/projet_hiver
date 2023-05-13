#pragma once

#include "CoreMinimal.h"
#include "ContractStatus.generated.h"

UENUM(BlueprintType)
enum class EContractStatus : uint8
{
	NotStarted,
	OnGoing,
	Completable,
	Succeeded,
	Failed
};
