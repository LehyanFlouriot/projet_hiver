#pragma once

#include "CoreMinimal.h"
#include "ReplicatedStaticMeshActor.generated.h"

UCLASS(Abstract)
class PROJETHIVER_API AReplicatedStaticMeshActor : public AActor
{
	GENERATED_BODY()

public:
	AReplicatedStaticMeshActor();

protected:
	UPROPERTY()
	USceneComponent* SceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* StaticMeshComponent;

public:
	UFUNCTION()
	UStaticMeshComponent* GetStaticMeshComponent() const noexcept;

	UFUNCTION()
	void SetMobility(EComponentMobility::Type InMobility) const;
};
