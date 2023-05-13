#include "RangeBaseProjectile.h"

#include "AbilitySystemComponent.h"
#include "ProjetHiver/Characters/BaseCharacter.h"

void ARangeBaseProjectile::ApplyProjectileEffects(const ABaseCharacter* Source, ABaseCharacter* Target)
{
	Target->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
}

void ARangeBaseProjectile::SetDamageEffectSpecHandle(const FGameplayEffectSpecHandle& NewDamageEffectSpecHandle)
{
	DamageEffectSpecHandle = NewDamageEffectSpecHandle;
}
