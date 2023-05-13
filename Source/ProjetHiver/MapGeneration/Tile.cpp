#include "Tile.h"

#include "Gizmo.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/AI/AIManager.h"
#include "ProjetHiver/AI/InfluenceMap/InfluenceMapManager.h"

ATile::ATile() {
	bReplicates = true;
}

TArray<UGizmo*> ATile::GetGizmos(EGizmoType Type) const noexcept {
	TArray<UGizmo*> ReturnTiles;

	TArray<UGizmo*> TileGizmos;
	GetComponents<UGizmo*>(TileGizmos, true);
	for (UGizmo* Gizmo : TileGizmos)
		if(Type == Gizmo->GetType())
			ReturnTiles.Add(Gizmo);	

	return ReturnTiles;
}

void ATile::Unload() const
{
	TArray<UGizmo*> TileGizmos;
	GetComponents<UGizmo*>(TileGizmos, true);
	for (UGizmo* Gizmo : TileGizmos)
	{
		if(IsValid(Gizmo))
			Gizmo->DestroySpawnedActor();
	}
	if (UInfluenceMapManager* Manager = GetWorld()->GetGameState<AEfhorisGameState>()->GetAIManager()->GetIMapManager(); IsValid(Manager)) {
		Manager->RemoveIMap(Manager->WorldLocationToGrid(GetActorLocation()));
	}
}
