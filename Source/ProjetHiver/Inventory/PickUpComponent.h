#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PickUpComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJETHIVER_API UPickUpComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPickUpComponent();

protected:
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;

private:
	UFUNCTION()
	void OnOwnerOverlap(AActor* OwningActor, AActor* OtherActor);
};
