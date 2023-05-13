#pragma once

#include "CoreMinimal.h"
#include "HealSeed.generated.h"

class UGameplayEffect;
class USphereComponent;

UCLASS()
class PROJETHIVER_API AHealSeed final : public AActor
{
	GENERATED_BODY()

public:
	AHealSeed();

	virtual void PostInitializeComponents() override;

protected:
	UPROPERTY()
	USceneComponent* SceneComponent;

	UPROPERTY(EditAnywhere)
	USphereComponent* SphereComponent;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> HealSeedEffect;

	UFUNCTION()
	void OnCapsuleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	UFUNCTION()
	USphereComponent* GetSphere() const { return SphereComponent; }
};
