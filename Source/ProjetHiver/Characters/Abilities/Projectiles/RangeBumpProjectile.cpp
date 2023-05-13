#include "ProjetHiver/Characters/Abilities/Projectiles/RangeBumpProjectile.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjetHiver/Characters/BaseCharacter.h"
#include "ProjetHiver/Characters/BaseNPC.h"
#include "ProjetHiver/MapGeneration/TileLimit.h"

void ARangeBumpProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA<ATileLimit>()) return;
	if (GetInstigator() == OtherActor) return;

	const ABaseCharacter* Source = Cast<ABaseCharacter>(GetInstigator());
	ABaseCharacter* Target = Cast<ABaseCharacter>(OtherActor);

	if (IsValid(Source) && IsValid(Target) && Source->GetFactionTag() != Target->GetFactionTag() && Target->IsAliveHealth() && !Target->IsBumped())
	{
		if (!Target->GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsInvincible"))))
		{
			if (!(Target->GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.SwordShield.Parry"))) && Target->IsSuccessfullyBlocking(this)))
				ApplyProjectileEffects(Source, Target);
		}
		// If invincible & dodging
		else if (Target->GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Move.Dodge")))) return;

		Destroy();
	}
	// Otherwise, we destroy the projectile only if the OtherActor is not a ABaseCharacter.
	// It means that the projectile will be destroyed if it hits the environment but will go through allied characters.
	else if (!IsValid(Target)) Destroy();
}

void ARangeBumpProjectile::ApplyProjectileEffects(const ABaseCharacter* Source, ABaseCharacter* Target)
{
	Target->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());

	//Bump target
	if (HasAuthority() && Target->IsA<ABaseNPC>())
	{
		FVector RepulsionDirection = Target->GetActorLocation() - Source->GetActorLocation();
		RepulsionDirection.Normalize();

		FVector BumpDirection = (RepulsionDirection + FVector::UpVector * UpCoefficient);
		BumpDirection.Normalize();

		Target->LaunchCharacter(BumpDirection * BumpForce, true, true);
		Target->SetIsBumped(true);
	}
}

void ARangeBumpProjectile::SetDamageEffectSpecHandle(const FGameplayEffectSpecHandle& NewDamageEffectSpecHandle)
{
	DamageEffectSpecHandle = NewDamageEffectSpecHandle;
}

void ARangeBumpProjectile::SetBumpForce(const float NewBumpForce)
{
	BumpForce = NewBumpForce;
}

void ARangeBumpProjectile::SetUpCoefficient(const float NewUpCoefficient)
{
	UpCoefficient = NewUpCoefficient;
}
