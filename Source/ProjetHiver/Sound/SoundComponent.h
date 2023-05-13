// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SoundComponent.generated.h"

class UAkAudioEvent;
enum class AkActionOnEventType : uint8;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJETHIVER_API USoundComponent final : public USceneComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient)
	TMap<UAkAudioEvent*, int32> SoundMap;

public:	
	// Sets default values for this component's properties
	USoundComponent();

	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;

	UFUNCTION(BlueprintCallable)
	void PlaySound(UAkAudioEvent* Event, AActor* Actor = nullptr) const;

	UFUNCTION(BlueprintCallable)
	void PlaySoundAtLocation(UAkAudioEvent* Event, const FVector Location, const FRotator Rotation) const;

	UFUNCTION(BlueprintCallable)
	void PlaySoundAttached(UAkAudioEvent* Event, AActor* Actor = nullptr) const;

	UFUNCTION(BlueprintCallable)
	void PlayStoppableSound(UAkAudioEvent* Event, AActor* Actor = nullptr);

	UFUNCTION(BlueprintCallable)
	void PlayStoppableSoundAtLocation(UAkAudioEvent* Event, const FVector Location, const FRotator Rotation);

	UFUNCTION(BlueprintCallable)
	void PlayStoppableSoundAttached(UAkAudioEvent* Event, AActor* Actor = nullptr);

	UFUNCTION(BlueprintCallable)
	void ActOnSound(UAkAudioEvent* Event, const AkActionOnEventType Action);
};
