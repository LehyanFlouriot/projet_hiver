#include "ReplicatedStaticMeshActor.h"

AReplicatedStaticMeshActor::AReplicatedStaticMeshActor()
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>("Root");
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("Static Mesh"));

	SetRootComponent(SceneComponent);
	StaticMeshComponent->SetupAttachment(SceneComponent);

	bReplicates = true;
	AActor::SetReplicateMovement(true);
	SetMobility(EComponentMobility::Movable);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

UStaticMeshComponent* AReplicatedStaticMeshActor::GetStaticMeshComponent() const noexcept
{
	return StaticMeshComponent;
}

void AReplicatedStaticMeshActor::SetMobility(const EComponentMobility::Type InMobility) const
{
	StaticMeshComponent->SetMobility(InMobility);
}
