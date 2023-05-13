#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Gizmo.generated.h"

UENUM(BlueprintType)
enum class EGizmoType : uint8
{
	Chest,
	Barrel,
	AloneEnemy,
	SubBoss,
	Boss,
	QuestItem,
	Merchant,
	SafeZoneMerchant,
	Kevin,
	Mercenary
};
ENUM_RANGE_BY_FIRST_AND_LAST(EGizmoType, EGizmoType::Chest, EGizmoType::Mercenary);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJETHIVER_API UGizmo : public USceneComponent
{
	GENERATED_BODY()

public:
	EGizmoType GetType() const noexcept { return Type; }
	void SetType(EGizmoType T) noexcept { Type = T; }

	AActor* GetSpawnedActor() { return SpawnedActor; }
	void SetSpawnedActor(AActor* Actor) { SpawnedActor = Actor; }
	void DestroySpawnedActor();

private:
	UPROPERTY(EditAnywhere, Category = "Type")
	EGizmoType Type;
	
	/** Actor Spawned at the gizmo position. */
	UPROPERTY(Transient)
	AActor* SpawnedActor;
};
