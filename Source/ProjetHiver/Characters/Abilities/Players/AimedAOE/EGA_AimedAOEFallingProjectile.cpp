#include "EGA_AimedAOEFallingProjectile.h"

#include "ProjetHiver/Characters/BaseCharacter.h"
#include "ProjetHiver/Characters/Abilities/Projectiles/BaseAOEProjectile.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"

void UEGA_AimedAOEFallingProjectile::ExecuteAOE(const FGameplayAbilityTargetDataHandle& Data)
{
	Super::ExecuteAOE(Data);

	AoELocation.Z += ProjectileSpawnHeight;

	if (CastGameplayCueTag.IsValid())
	{
		FGameplayCueParameters GameplayCueParameters;
		GameplayCueParameters.Location = AoELocation;
		K2_ExecuteGameplayCueWithParams(CastGameplayCueTag, GameplayCueParameters);
	}

	if (HasAuthority(&CurrentActivationInfo))
		if (ABaseCharacter* Source = Cast<ABaseCharacter>(GetOwningActorFromActorInfo()))
		{
			FTransform Transform;
			Transform.SetLocation(AoELocation);

			ABaseAOEProjectile* Projectile = CastChecked<AEfhorisGameMode>(GetWorld()->GetAuthGameMode())
				->SpawnActorDeferred<ABaseAOEProjectile>(ProjectileClass, Transform, GetOwningActorFromActorInfo(), Source,
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			Projectile->SetDamageEffectSpecHandle(CreateDamageEffect(DamageEffect, DamageInfo));
			Projectile->SetRange(0); // The actor will only be destroyed when hitting the ground so its lifetime should be infinite.
			Projectile->SetAOERadius(SphereInfo.SphereRadius);
			Projectile->FinishSpawning(Transform);

			bShouldEndAbility = true;
		}
}
