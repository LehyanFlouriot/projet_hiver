#include "BaseAOEProjectile.h"

#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProjetHiver/Characters/BaseCharacter.h"
#include "ProjetHiver/MapGeneration/TileLimit.h"

void ABaseAOEProjectile::ApplyProjectileEffects(const ABaseCharacter* Source, ABaseCharacter* Target)
{
	Target->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
}

void ABaseAOEProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA<ATileLimit>()) return;
	if (GetInstigator() == OtherActor) return;

	if (OtherComp->GetCollisionObjectType() == ECC_WorldStatic || OtherComp->GetCollisionObjectType() == ECC_WorldDynamic)
	{
		if (const ABaseCharacter* Source = Cast<ABaseCharacter>(GetInstigator()))
		{
			TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
			ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

			TArray<AActor*> OutActors;

			FColor Color;

			if (UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), AOERadius, ObjectTypes, ABaseCharacter::StaticClass(), {}, OutActors))
			{
				for (AActor* Actor : OutActors)
					if (ABaseCharacter* Target = Cast<ABaseCharacter>(Actor); IsValid(Target))
						if (Source->GetFactionTag() != Target->GetFactionTag() &&
							Target->IsAliveHealth() &&
							!Target->GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsInvincible"))))
								ApplyProjectileEffects(Source, Target);

				Color = FColor::Green;
			}
			else
				Color = FColor::Red;

			if (Source->GetDebugAbilitiesHitbox())
				DrawDebugSphere(GetWorld(), GetActorLocation(), AOERadius, 10, Color, false, 3.f);
		}

		Destroy();
	}
}

void ABaseAOEProjectile::SetDamageEffectSpecHandle(const FGameplayEffectSpecHandle& NewDamageEffectSpecHandle)
{
	DamageEffectSpecHandle = NewDamageEffectSpecHandle;
}

void ABaseAOEProjectile::SetAOERadius(const float NewRadius)
{
	AOERadius = NewRadius;
}
