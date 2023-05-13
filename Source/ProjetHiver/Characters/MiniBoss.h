#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Characters/Enemy.h"
#include "MiniBoss.generated.h"

UCLASS()
class PROJETHIVER_API AMiniBoss : public AEnemy
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = GAS)
	TSubclassOf<UGameplayEffect> MiniBossModifier;

	virtual void BeginPlay() override;
	virtual void Die() override;
};
