#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Characters/Abilities/Players/AimedAOE/EGA_AimedAOETick.h"
#include "EGA_BlackHole.generated.h"

class ABaseCharacter;
class UAbilityTask_Repeat;

UCLASS()
class PROJETHIVER_API UEGA_BlackHole final : public UEGA_AimedAOETick
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	float PullStrength;

protected:
	virtual void ApplyEffectToAll(const ABaseCharacter* Source, ABaseCharacter* Target) override;
};
