#include "Gizmo.h"

void UGizmo::DestroySpawnedActor()
{
	if (IsValid(SpawnedActor))
		SpawnedActor->K2_DestroyActor();
}
