#include "InfluenceMapManager.h"

#include "ProjetHiver/AI/InfluenceMap/InfluenceMap.h"
#include "ProjetHiver/AI/InfluenceMap/TemplateManager.h"

bool UInfluenceMapManager::IsInBound(const FIntVector2& GridCoord) const noexcept {
	return FMath::Abs(GridCoord.X) < MaxTileDist && FMath::Abs(GridCoord.Y) < MaxTileDist;
}

//Returns the IMap grid coordinates (X,Y) corresponding to the world location Location.
FIntVector2 UInfluenceMapManager::WorldLocationToGrid(const FVector& Location) const {
	const int X = FMath::Floor((Location.X + TileSize / 2) / TileSize);
	const int Y = FMath::Floor((Location.Y + TileSize / 2) / TileSize);
	return FIntVector2(X,Y);
}

//Finds InfluenceMaps that intersect IMap when it is placed at Start. If bAllowIMapGeneration, then will create new InfluenceMaps when necessary.
TArray<UInfluenceMap*> UInfluenceMapManager::FindConcernedIMaps(const UInfluenceMap* IMap, const FVector& Start, const bool bAllowIMapGeneration) {
	TArray<FVector> Corners = IMap->GetCornersOffsetFromStart(Start);
	TArray<UInfluenceMap*> FoundMaps;
	for (const FVector& Corner : Corners) {
		const FIntVector2 XY = WorldLocationToGrid(Corner);
		if (UInfluenceMap** PIMap = Map.Find(XY)) {
			if (!FoundMaps.Contains(*PIMap)) {
				FoundMaps.Add(*PIMap);
			}
		}
		else {
			if (bAllowIMapGeneration) {
				if (UInfluenceMap* NewMap = NewIMap(XY)) {
					FoundMaps.Add(NewMap);
				}
			}
		}
	}
	return FoundMaps;
}

//Creates a new IMap, and adds it at location GridCoord.
UInfluenceMap* UInfluenceMapManager::NewIMap(const FIntVector2& GridCoord) {
	if (IsInBound(GridCoord) && !Map.Contains(GridCoord)) {
		UInfluenceMap* IMap = NewObject<UInfluenceMap>();
		IMap->Init(Resolution, GridCoord.X, GridCoord.Y,DecayRate);
		Map.Add(GridCoord, IMap);
		return IMap;
	}
	return nullptr;
}

void UInfluenceMapManager::RemoveIMap(const FIntVector2& GridCoord) {
	Map.Remove(GridCoord);
}

void UInfluenceMapManager::Init() {
	if (TemplateManagerClass) {
		UTemplateManager::InitStatic(TileSize,Resolution,DisplayHeight);
		TemplateManager = NewObject<UTemplateManager>(NewObject<UTemplateManager>(), TemplateManagerClass);
		if (TemplateManager) {
			TemplateManager->Init();
		}
	}
}

void UInfluenceMapManager::Tick(const float& DeltaTime) {
	for(const auto& Pair : Map) {
		if (Pair.Value) {
			Pair.Value->Tick(DeltaTime);
		}
	}
}

//Adds Imap at Location with a factor of Magnitude. Creates map if location is empty.
void UInfluenceMapManager::AddAt(const UInfluenceMap* IMap, const FVector& Start, const float& Magnitude) {
	TArray<UInfluenceMap*> FoundMaps = FindConcernedIMaps(IMap, Start,true);
	for (UInfluenceMap* FoundMap : FoundMaps) {
		FIntVector2 InMapXY = FoundMap->ToInMapXY(Start);
		FoundMap->AddAt(IMap, InMapXY, Magnitude);
	}
}

void UInfluenceMapManager::AddAtCenter(const UInfluenceMap* IMap, const FVector& Center, const float& Magnitude) {
	const float Delta = IMap->GetSize() * IMap->GetCellSize() / 2.0f;
	AddAt(IMap, FVector(Center.X - Delta, Center.Y - Delta, Center.Z), Magnitude);
}

void UInfluenceMapManager::ApplyAt(const UInfluenceMap* IMap, const FVector& Start) {
	TArray<UInfluenceMap*> FoundMaps = FindConcernedIMaps(IMap, Start, false);
	for (UInfluenceMap* FoundMap : FoundMaps) {
		FIntVector2 InMapXY = FoundMap->ToInMapXY(Start);
		FoundMap->ApplyAt(IMap, InMapXY);
	}
}

void UInfluenceMapManager::ApplyAtCenter(const UInfluenceMap* IMap, const FVector& Center) {
	const float Delta = IMap->GetSize() * IMap->GetCellSize() / 2.0f;
	ApplyAt(IMap, FVector(Center.X - Delta, Center.Y - Delta, Center.Z));
}

UInfluenceMap* UInfluenceMapManager::GetIMap(const FVector& WorldLocation, const bool bCanCreateNewMap) {
	return GetIMap(WorldLocationToGrid(WorldLocation),bCanCreateNewMap);
}

UInfluenceMap* UInfluenceMapManager::GetIMap(const FIntVector2& GridCoord, const bool bCanCreateNewMap) {
	if (IsInBound(GridCoord)) {
		if (const auto PIMap = Map.Find(GridCoord)) {
			return *PIMap;
		}
		else if (bCanCreateNewMap) {
			return NewIMap(GridCoord);
		}
	}
	return nullptr;
}

void UInfluenceMapManager::SetIMap(UInfluenceMap* IMap, const FIntVector2& GridCoord) {
	if (IsInBound(GridCoord)) {
		Map.Add(GridCoord,IMap);
	}
}

UInfluenceMap* UInfluenceMapManager::GetTemplate(const ETemplateType& TemplateType) const {
	if (TemplateManager)
		return TemplateManager->GetTemplate(TemplateType);
	else 
		return nullptr;
}

//DEBUG

void UInfluenceMapManager::DrawAll(const UWorld* World, const float& DeltaSeconds) const {
	for(const auto& Pair : Map){
		if (Pair.Value) {
			Pair.Value->Draw(World, DeltaSeconds);
		}
	}
}

void UInfluenceMapManager::DrawIMap(const UWorld* World, const float& DeltaSeconds,const FIntVector2& GridCoord) {
	if (const UInfluenceMap* IMap = GetIMap(GridCoord)) {
		IMap->Draw(World,DeltaSeconds);
	}
}