#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class PROJETHIVER_API ABaseProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseProjectile();

protected:
	UPROPERTY(Transient)
	float Range;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	USphereComponent* Sphere;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere)
	FGameplayTag DestructionGameplayCueTag;

	UPROPERTY(EditAnywhere)
	bool bDestroyAfterEffectApplication = true;

	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void ApplyProjectileEffects(const ABaseCharacter* Source, ABaseCharacter* Target) {}

public:
	virtual void PostInitializeComponents() override;
	virtual void Destroyed() override;

	void SetRange(const float NewRange);
};
