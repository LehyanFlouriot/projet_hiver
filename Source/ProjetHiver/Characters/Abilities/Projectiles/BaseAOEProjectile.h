#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "ProjetHiver/Characters/Abilities/Projectiles/BaseProjectile.h"
#include "BaseAOEProjectile.generated.h"

UCLASS()
class PROJETHIVER_API ABaseAOEProjectile final : public ABaseProjectile
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)
	FGameplayEffectSpecHandle DamageEffectSpecHandle;

	UPROPERTY(Transient)
	float AOERadius;

	virtual void ApplyProjectileEffects(const ABaseCharacter* Source, ABaseCharacter* Target) override;

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

public:
	void SetDamageEffectSpecHandle(const FGameplayEffectSpecHandle& NewDamageEffectSpecHandle);
	void SetAOERadius(const float NewRadius);
};
