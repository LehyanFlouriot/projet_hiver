#include "TemplateManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProjetHiver/AI/InfluenceMap/InfluenceMap.h"

void FInfluenceCurve::Init(const ECurve& Curve, const int Outer, const int Inner) {
	CurveType = Curve;
	OuterRadius = Outer;
	InnerRadius = Inner;
}

float FInfluenceCurve::ValueAt(const int DistToCenter) const {
	float D;
	switch (CurveType)
	{
	case ECurve::Constant:
		return 1.0f;
	case ECurve::Linear:
		return FMath::Max(1.0f - DistToCenter / static_cast<float>(OuterRadius), 0.0f);
	case ECurve::Quadratic:
		D = DistToCenter / static_cast<float>(OuterRadius);
		return FMath::Max(1.0f - D * D, 0.0f);
	case ECurve::Disc:
		return DistToCenter <= InnerRadius ? -FMath::Max(1.0f - DistToCenter / static_cast<float>(InnerRadius), 0.0f) : DistToCenter < OuterRadius ? 1.0f : 0.0f;
	default:
		return 0.0f;
	}
}

int UTemplateManager::RadiusToIMapSize(const float& Radius) {
	return 1 + 2.0f * Radius / UInfluenceMap::GetCellSize();
}

int UTemplateManager::RadiusToCurveRange(const float& Radius) {
	return Radius / UInfluenceMap::GetCellSize();
}
//Should be called on an empty Imap
UInfluenceMap* UTemplateManager::Propagate(const FInfluenceCurve& Curve, const int& IMapSize) const {
	UInfluenceMap* Template = NewObject<UInfluenceMap>();
	Template->Init(IMapSize);
	const FIntVector2 CenterXY = FIntVector2(IMapSize / 2, IMapSize / 2);
	for (int I = 0; I < IMapSize * IMapSize; I++) {
		const FIntVector2 XY = Template->XYFromI(I);
		const int Distance = UKismetMathLibrary::Sqrt(FMath::Square(FMath::Abs(CenterXY.X - XY.X)) + FMath::Square(FMath::Abs(CenterXY.Y - XY.Y))); //Calcul de distance
		Template->Data[I] = Curve.ValueAt(Distance);
	}
	return Template;
}

UInfluenceMap* UTemplateManager::MakeTemplate(const ECurve& Curve, const float& OuterRadius, const float& InnerRadius) const {
	FInfluenceCurve Curve_;
	Curve_.Init(Curve, RadiusToCurveRange(OuterRadius),RadiusToCurveRange(InnerRadius));
	return Propagate(Curve_, RadiusToIMapSize(OuterRadius));
}

void UTemplateManager::InitStatic(const float& TileSize, const float& Resolution, const float& DisplayHeight) {
	UInfluenceMap::TileSize = TileSize;
	UInfluenceMap::CellSize = TileSize / Resolution;
	UInfluenceMap::DisplayHeight = DisplayHeight;
}

void UTemplateManager::Init() {
	AllyTemplate = MakeTemplate(AllyInfluenceType, AllyInfluenceRadius);
	PlayerMeleeTemplate = MakeTemplate(PlayerMeleeInfluenceType, PlayerMeleeOuterInfluenceRadius, PlayerMeleeInnerInfluenceRadius);
	PlayerDistanceTemplate = MakeTemplate(PlayerDistanceInfluenceType, PlayerDistanceOuterInfluenceRadius, PlayerDistanceInnerInfluenceRadius);
	EnemyTemplate = MakeTemplate(EnemyInfluenceType, EnemyInfluenceRadius);
	MovementTemplate = MakeTemplate(MovementInfluenceType, MovementInfluenceRadius);
}

UInfluenceMap* UTemplateManager::GetTemplate(const ETemplateType& TemplateType) const {
	switch (TemplateType) {
	case ETemplateType::AllyUnit :
		return AllyTemplate;
	case ETemplateType::PlayerMelee :
		return PlayerMeleeTemplate;
	case ETemplateType::PlayerDistance :
		return PlayerDistanceTemplate;
	case ETemplateType::Enemy :
		return EnemyTemplate;
	case ETemplateType::Movement :
		return MovementTemplate;
	default :
		return nullptr;
	}
}
