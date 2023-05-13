#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "Wolf.generated.h"

class AGroup;

UCLASS()
class PROJETHIVER_API AWolf : public AEnemy
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)
	AGroup* Group;

	virtual void ClearUnitReferences() override;

public:
	void SetGroup(AGroup* Group_);
};
