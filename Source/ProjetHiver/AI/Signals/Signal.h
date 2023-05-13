// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Signal.generated.h"

class ABaseCharacter;
UENUM(BlueprintType)
enum class ESignalType : uint8
{
	PlayerDied,
	PlayerAbilityCast,
	TargetHit,
	NewTargetSensed,
	None
};

/**
 * 
 */
UCLASS()
class PROJETHIVER_API USignal final : public UObject
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	ESignalType SignalType;

	UPROPERTY(Transient)
	float Radius;

	UPROPERTY(Transient)
	ABaseCharacter* Emitter;

public:
	UFUNCTION()
	void Init(ESignalType SignalType_, float Radius_, ABaseCharacter* Emitter_);

	UFUNCTION()
	ESignalType GetSignalType() const noexcept { return SignalType; }

	UFUNCTION()
	ABaseCharacter* GetEmitter() const noexcept { return Emitter; }

	UFUNCTION()
	float GetRadius() const noexcept { return Radius; }

	UFUNCTION()
	void SetSignalType(ESignalType SignalType_) noexcept { SignalType = SignalType_; }

	UFUNCTION()
	void SetRadius(float Radius_) noexcept { Radius = Radius_; }

	UFUNCTION()
	void SetEmitter(ABaseCharacter* Emitter_) noexcept { Emitter = Emitter_; }

};