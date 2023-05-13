#pragma once

#include "CoreMinimal.h"
#include "SphereInfo.generated.h"

USTRUCT(BlueprintType)
struct FSphereInfo 
{
	GENERATED_BODY()

	FSphereInfo() : SphereRadius(0.f) {}

	UPROPERTY(EditAnywhere)
	float SphereRadius;

	UPROPERTY(EditAnywhere)
	FVector SphereOffset = FVector(0);
};
