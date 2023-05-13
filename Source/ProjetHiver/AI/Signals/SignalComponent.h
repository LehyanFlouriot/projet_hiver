// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ProjetHiver/AI/Signals/Signal.h"
#include "SignalComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJETHIVER_API USignalComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USignalComponent();

	UFUNCTION()
	void SendSignal(USignal* Signal);

	UFUNCTION()
	void ReceiveSignal(USignal* Signal) const;

	UFUNCTION()
	void SetShowSignals(bool bShow);

	UFUNCTION()
	bool GetShowSignals() const noexcept { return bShowSignals; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(Replicated, Transient)
	bool bShowSignals;

	UPROPERTY(EditAnywhere)
	bool bCanReceiveSignals;

	UPROPERTY(EditAnywhere)
	bool bCanSendSignals;

	UFUNCTION()
	void ReceiveSignalTargetSensed(ABaseCharacter* Emitter) const;

	UFUNCTION()
	void ReceiveSignalPlayerDied(ABaseCharacter* Emitter) const;

	UFUNCTION()
	void ReceiveSignalPlayerAbilityCast(ABaseCharacter* Emitter) const;

};
