// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeightedActor.generated.h"

/**
 *  Represent an Actor with it weight function associated
 */
UCLASS(BlueprintType)
class PROJETHIVER_API AWeightedActor : public AActor
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	float WeightFct(float Difficulty);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ActorClass;
};
