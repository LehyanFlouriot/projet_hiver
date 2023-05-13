#pragma once

#include "CoreMinimal.h"
#include "ReplicatedStaticMeshActor.h"
#include "ProjetHiver/Items/ItemInfo.h"
#include "PickableItem.generated.h"

class UBoxComponent;
class UNiagaraComponent;

UCLASS()
class PROJETHIVER_API APickableItem final : public AReplicatedStaticMeshActor
{
	GENERATED_BODY()

public:
	APickableItem();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBoxComponent* BoxComponent;

	UPROPERTY(EditAnywhere)
	UNiagaraComponent* GroundFX;

	UPROPERTY(EditAnywhere)
	UNiagaraComponent* BeamFX;

	UPROPERTY(Transient,Replicated)
	FItemInfo ItemInfo;

	UPROPERTY(Transient)
	bool bIsPickable = false;

	UPROPERTY(EditAnywhere)
	float AnimationFrequency =1.0f;

	UPROPERTY(EditAnywhere)
	float AnimationAmplitude = 1.0f;

	UPROPERTY(EditAnywhere)
	float AnimationRotationSpeed = 1.0f;

	UPROPERTY(Transient)
	float Time;

	UPROPERTY(Transient)
	FTransform InitialTransform;

	UPROPERTY(EditAnywhere)
	TArray<FLinearColor> ColorLevel;

	UPROPERTY(EditAnywhere)
	float EmissionIntensity = 5.0f;

	UPROPERTY(Transient)
	bool bIsAnimated;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	FLinearColor GetColorLevel(int Level) const;

	void SetFXColor() const;

	void ActivateFX(const bool bIsActive);

	void StartAnimation();

public:
	inline FItemInfo& GetItemInfo() noexcept;
	inline void SetItemInfo(const FItemInfo& NewItemInfo) noexcept;

	UBoxComponent* GetCollision() const noexcept;

	bool IsPickable() const noexcept;
	void SetIsPickable(const bool bNewIsPickable) noexcept;
};
