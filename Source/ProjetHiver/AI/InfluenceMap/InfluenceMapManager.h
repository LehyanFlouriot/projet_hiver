#pragma once

#include "CoreMinimal.h"
#include "InfluenceMapManager.generated.h"

class UInfluenceMap;
class UTemplateManager;
enum class ETemplateType;

//enum class EIMapType : uint8 {
//	Spells,
//	Units,
//	Movement,
//};

UCLASS(Blueprintable)
class PROJETHIVER_API UInfluenceMapManager : public UObject
{
	GENERATED_BODY()

	friend class UTemplateManager;

private:
	UPROPERTY(Transient)
	TMap<FIntVector2, UInfluenceMap*> Map;

	UPROPERTY(EditAnywhere)
	float TileSize = 6000.0f;

	UPROPERTY(EditAnywhere)
	int Resolution = 30;

	UPROPERTY(EditAnywhere)
	int MaxTileDist = 10;

	//Should be between 0 and 1
	UPROPERTY(EditAnywhere)
	float DecayRate = 0.95f;

	UPROPERTY(EditAnywhere)
	float DisplayHeight = 150.0f;

	UPROPERTY(EditAnywhere, NoClear)
	TSubclassOf<UTemplateManager> TemplateManagerClass;

	UPROPERTY(Transient)
	UTemplateManager* TemplateManager;

	UFUNCTION()
	bool IsInBound(const FIntVector2& GridCoord) const noexcept;

	UFUNCTION()
	TArray<UInfluenceMap*> FindConcernedIMaps(const UInfluenceMap* IMap, const FVector& Start, const bool bAllowIMapGeneration = true);

	UFUNCTION()
	UInfluenceMap* NewIMap(const FIntVector2& GridCoord);

public:
	UFUNCTION()
	void Init();

	UFUNCTION()
	void Tick(const float& DeltaTime);

	void RemoveIMap(const FIntVector2& GridCoord);

	UFUNCTION()
	FIntVector2 WorldLocationToGrid(const FVector& Location) const;

	//OPERATORS

	UFUNCTION()
	void AddAt(const UInfluenceMap* IMap, const FVector& Start, const float& Magnitude = 1.0f);

	UFUNCTION()
	void AddAtCenter(const UInfluenceMap* IMap, const FVector& Center, const float& Magnitude = 1.0f);

	UFUNCTION()
	void ApplyAt(const UInfluenceMap* IMap, const FVector& Start);

	UFUNCTION()
	void ApplyAtCenter(const UInfluenceMap* IMap, const FVector& Center);

	//GETTERS && SETTERS
	UInfluenceMap* GetIMap(const FVector& WorldLocation, const bool bCanCreateNewMap = false);
	UInfluenceMap* GetIMap(const FIntVector2& GridCoord, const bool bCanCreateNewMap = false);
	void SetIMap(UInfluenceMap* IMap, const FIntVector2& GridCoord);

	UInfluenceMap* GetTemplate(const ETemplateType& TemplateType) const;

	//DEBUG

	UFUNCTION()
	void DrawAll(const UWorld* World, const float& DeltaSeconds) const;

	UFUNCTION()
	void DrawIMap(const UWorld* World, const float& DeltaSeconds, const FIntVector2& GridCoord);
};