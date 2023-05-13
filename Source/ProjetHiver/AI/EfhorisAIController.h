#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EfhorisAIController.generated.h"

class UBehaviorTree;

UCLASS()
class PROJETHIVER_API AEfhorisAIController : public AAIController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void RunBehavior();

	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn) override;

	UPROPERTY(EditAnywhere)
	UBehaviorTree* Behavior;

protected:
	UFUNCTION()
	virtual void OnPossess(APawn* InPawn) override;
};
