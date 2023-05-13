#pragma once

#include "CoreMinimal.h"
#include "PullInfo.generated.h"

USTRUCT()
struct FPullInfo
{
	GENERATED_BODY()

	FPullInfo() : RowName(FName{}), PullNumber(1), Probability(1.f) {}

	UPROPERTY(EditAnywhere)
	FName RowName;

	UPROPERTY(EditAnywhere)
	int PullNumber;

	// Should be between 0 and 1.
	UPROPERTY(EditAnywhere)
	float Probability;
};