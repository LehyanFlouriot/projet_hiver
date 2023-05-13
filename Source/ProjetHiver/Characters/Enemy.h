#pragma once

#include "CoreMinimal.h"
#include "BaseNPC.h"
#include "Enemy.generated.h"

UENUM()
enum class ERoleFormation {
	Melee = 0,
	Hybrid = 1,
	Range = 2,
	None = 3,
};

class ULootComponent;
class USphereComponent;

UCLASS()
class PROJETHIVER_API AEnemy : public ABaseNPC
{
	GENERATED_BODY()

public:
	AEnemy(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditAnywhere)
	ULootComponent* LootComponent;

	UPROPERTY(Transient, EditAnywhere)
	USphereComponent* SphereComponent = nullptr;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Die() override;

	virtual void ClearUnitReferences() override;

	UPROPERTY(EditAnywhere)
	ERoleFormation ERole = ERoleFormation::None;

	UFUNCTION()
	FVector StandardMove() const;

	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
