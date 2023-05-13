#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ProjetHiver/LootComponent/LootTableRow.h"
#include "ProjetHiver/LootComponent/PullInfo.h"
#include "LootComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJETHIVER_API ULootComponent final : public USceneComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	TArray<FPullInfo> PullInfos;

	UPROPERTY(EditAnywhere)
	FVector LootSpawnOffset;

	UPROPERTY(EditAnywhere)
	float LootConeAngleDegree;

	UPROPERTY(EditAnywhere)
	float LootSpeed;

	UPROPERTY(EditAnywhere)
	int LootLevelOffset = 1;

	UPROPERTY(Transient)
	int Level;

	UPROPERTY(Transient)
	TArray<float> TotalWeights;

	UPROPERTY(Transient)
	TArray<FLootTableRow> LootTableRows;

	UPROPERTY(Transient)
	FTransform LootOrigin;

	UPROPERTY(EditAnywhere)
	FVector VelocityDirection = FVector(0,0,1);

	virtual void BeginPlay() override;

	UFUNCTION()
	AActor* GenerateItem(const EItemType Type, const FName& Name) const;

	UFUNCTION()
	void SetSpawnValues();

	UFUNCTION()
	static float CalculateTotalWeight(const TArray<FWeightedItem>& WeightedItems);

	UFUNCTION()
	void AddItemImpulse(const APickableItem* Item, const FVector& Impulse);

public:
	UFUNCTION()
	void GenerateLoot();
};
