#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Group.generated.h"

class AEnemy;

UCLASS(Blueprintable)
class PROJETHIVER_API AGroup : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere,NoClear)
	TSubclassOf<AWolf> WolfClass;

	UPROPERTY(Transient)
	TArray<AWolf*> Units;

	UPROPERTY(Transient)
	int Level = 1;

	UPROPERTY(Transient)
	int InitialUnitCount = 1;

	virtual void BeginPlay() override;

public:

	UFUNCTION()
	void SetLevel(const int Level_);

	UFUNCTION()
	void SetEnemyCount(const int Count);

	UFUNCTION()
	void AddUnit(AWolf* Unit);

	UFUNCTION()
	void RemoveUnit(AWolf* Unit);

	UFUNCTION()
	TArray<AWolf*>& GetUnits();

	virtual void K2_DestroyActor() override;
};