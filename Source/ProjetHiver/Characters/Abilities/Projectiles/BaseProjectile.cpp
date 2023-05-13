#include "BaseProjectile.h"

#include "AkComponent.h"
#include "AbilitySystemComponent.h"
#include "RangeBaseProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ProjetHiver/Characters/BaseCharacter.h"
#include "ProjetHiver/MapGeneration/TileLimit.h"

ABaseProjectile::ABaseProjectile()
{
	bReplicates = true;
	Sphere = CreateDefaultSubobject<USphereComponent>(FName("Capsule"));
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovement"));

	SetRootComponent(Sphere);
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(Range / ProjectileMovement->InitialSpeed);
}

void ABaseProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA<ATileLimit>()) return;
	if(OtherActor->IsA<ARangeBaseProjectile>()) return;
	if (GetInstigator() == OtherActor) return;

	const ABaseCharacter* Source = Cast<ABaseCharacter>(GetInstigator());
	ABaseCharacter* Target = Cast<ABaseCharacter>(OtherActor);

	if (IsValid(Source) && IsValid(Target) && Source->GetFactionTag() != Target->GetFactionTag() && Target->IsAliveHealth())
	{
		if (!Target->GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsInvincible"))))
		{
			if (!(Target->GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.SwordShield.Parry"))) && Target->IsSuccessfullyBlocking(this)))
				ApplyProjectileEffects(Source, Target);
		}
		// If invincible & dodging
		else if (Target->GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Move.Dodge")))) return;

		if (bDestroyAfterEffectApplication)
			Destroy();
	}
	// Otherwise, we destroy the projectile only if the OtherActor is not a ABaseCharacter.
	// It means that the projectile will be destroyed if it hits the environment but will go through allied characters.
	else if (!IsValid(Target)) Destroy();
}

void ABaseProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &ABaseProjectile::OnSphereOverlap);
}

void ABaseProjectile::Destroyed()
{
	Super::Destroyed();

	if (const ABaseCharacter* Source = Cast<ABaseCharacter>(GetInstigator()); IsValid(Source) && DestructionGameplayCueTag.IsValid())
	{
		FGameplayCueParameters GameplayCueParameters;
		GameplayCueParameters.Location = GetActorLocation();
		Source->GetAbilitySystemComponent()->ExecuteGameplayCue(DestructionGameplayCueTag, GameplayCueParameters);
	}
}

void ABaseProjectile::SetRange(const float NewRange)
{
	Range = NewRange;
}
