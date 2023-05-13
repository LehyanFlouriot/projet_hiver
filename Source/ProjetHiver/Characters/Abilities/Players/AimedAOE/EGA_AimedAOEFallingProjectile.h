#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Characters/Abilities/Players/AimedAOE/EGA_AimedAOEBase.h"
#include "EGA_AimedAOEFallingProjectile.generated.h"

class ABaseAOEProjectile;

UCLASS()
class PROJETHIVER_API UEGA_AimedAOEFallingProjectile final : public UEGA_AimedAOEBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseAOEProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere)
	float ProjectileSpawnHeight;

	UPROPERTY(EditAnywhere)
	FGameplayTag CastGameplayCueTag;

protected:
	virtual void ExecuteAOE(const FGameplayAbilityTargetDataHandle& Data) override;
};
