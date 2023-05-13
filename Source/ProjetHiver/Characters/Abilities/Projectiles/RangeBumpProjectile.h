#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "ProjetHiver/Characters/Abilities/Projectiles/BaseProjectile.h"
#include "RangeBumpProjectile.generated.h"

UCLASS()
class PROJETHIVER_API ARangeBumpProjectile : public ABaseProjectile
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)
	FGameplayEffectSpecHandle DamageEffectSpecHandle;

	UPROPERTY(Transient)
	float BumpForce;

	UPROPERTY(Transient)
	float UpCoefficient;

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	virtual void ApplyProjectileEffects(const ABaseCharacter* Source, ABaseCharacter* Target) override;

public:
	void SetDamageEffectSpecHandle(const FGameplayEffectSpecHandle& NewDamageEffectSpecHandle);
	void SetBumpForce(const float NewBumpForce);
	void SetUpCoefficient(const float NewUpCoefficient);
};
