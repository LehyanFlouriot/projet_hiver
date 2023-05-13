#pragma once

#include "CoreMinimal.h"
#include "TemplateManager.generated.h"

class UInfluenceMap;

UENUM()
enum class ECurve {
	Constant,
	Linear,
	Quadratic,
	Disc,
};

UENUM()
enum class ETemplateType {
	AllyUnit,
	PlayerMelee,
	PlayerDistance,
	Enemy,
	Movement,
};

USTRUCT()
struct FInfluenceCurve
{
	GENERATED_BODY()

	int InnerRadius;
	int OuterRadius;
	ECurve CurveType;

	void Init(const ECurve& Curve, const int Outer, const int Inner = 0);

	float ValueAt(const int DistToCenter) const;
};

UCLASS(Blueprintable)
class PROJETHIVER_API UTemplateManager : public UObject
{
	GENERATED_BODY()

private:
	//Player or Mercenary
	UPROPERTY(EditAnywhere,NoClear)
	float AllyInfluenceRadius = 500.0f;

	UPROPERTY(EditAnywhere,NoClear)
	ECurve AllyInfluenceType = ECurve::Linear;

	UPROPERTY(Transient)
	UInfluenceMap* AllyTemplate;

	//PlayerMelee
	//InnerRadius should be smaller than OuterRadius
	UPROPERTY(EditAnywhere, NoClear)
	float PlayerMeleeInnerInfluenceRadius = 300.0f;

	//OuterRadius should be bigger than InnerRadius
	UPROPERTY(EditAnywhere, NoClear)
	float PlayerMeleeOuterInfluenceRadius = 600.0f;

	UPROPERTY(VisibleAnywhere)
	ECurve PlayerMeleeInfluenceType = ECurve::Disc;

	UPROPERTY(Transient)
	UInfluenceMap* PlayerMeleeTemplate;

	//PlayerDistance
	//InnerRadius should be smaller than OuterRadius
	UPROPERTY(EditAnywhere, NoClear)
	float PlayerDistanceInnerInfluenceRadius = 1500.0f;

	//OuterRadius should be bigger than InnerRadius
	UPROPERTY(EditAnywhere, NoClear)
	float PlayerDistanceOuterInfluenceRadius = 2000.0f;

	UPROPERTY(VisibleAnywhere)
	ECurve PlayerDistanceInfluenceType = ECurve::Disc;

	UPROPERTY(Transient)
	UInfluenceMap* PlayerDistanceTemplate;

	//Enemy
	UPROPERTY(EditAnywhere, NoClear)
	float EnemyInfluenceRadius = 300.0f;

	UPROPERTY(EditAnywhere, NoClear)
	ECurve EnemyInfluenceType = ECurve::Quadratic;

	UPROPERTY(Transient)
	UInfluenceMap* EnemyTemplate;

	//Movement
	UPROPERTY(EditAnywhere, NoClear)
	float MovementInfluenceRadius = 1000.0f;

	UPROPERTY(EditAnywhere, NoClear)
	ECurve MovementInfluenceType = ECurve::Linear;

	UPROPERTY(Transient)
	UInfluenceMap* MovementTemplate;

	UFUNCTION()
	static int RadiusToIMapSize(const float& Radius);

	UFUNCTION()
	static int RadiusToCurveRange(const float& Radius);

	UFUNCTION()
	UInfluenceMap* Propagate(const FInfluenceCurve& Curve, const int& IMapSize) const;

	UFUNCTION()
	UInfluenceMap* MakeTemplate(const ECurve& Curve, const float& OuterRadius, const float& InnerRadius = 0.0f) const;

public:
	UFUNCTION()
	static void InitStatic(const float& TileSize, const float& Resolution, const float& DisplayHeight);

	UFUNCTION()
	void Init();

	UFUNCTION()
	UInfluenceMap* GetTemplate(const ETemplateType& TemplateType) const;
};
