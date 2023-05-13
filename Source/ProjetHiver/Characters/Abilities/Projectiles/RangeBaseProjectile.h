#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "ProjetHiver/Characters/Abilities/Projectiles/BaseProjectile.h"
#include "RangeBaseProjectile.generated.h"

UCLASS()
class PROJETHIVER_API ARangeBaseProjectile : public ABaseProjectile
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)
	FGameplayEffectSpecHandle DamageEffectSpecHandle;

	virtual void ApplyProjectileEffects(const ABaseCharacter* Source, ABaseCharacter* Target) override;

public:
	void SetDamageEffectSpecHandle(const FGameplayEffectSpecHandle& NewDamageEffectSpecHandle);
};
