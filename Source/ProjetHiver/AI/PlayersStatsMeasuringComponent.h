#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PlayersStatsMeasuringComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJETHIVER_API UPlayersStatsMeasuringComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayersStatsMeasuringComponent();

	void Authority_AddDamageTaken(double Damage) const;

protected:
	virtual void InitializeComponent() override;

	virtual void UninitializeComponent() override;

private:
	UFUNCTION()
	void OnOwnerEndOverlap(AActor* OwningActor, AActor* OtherActor);
};
