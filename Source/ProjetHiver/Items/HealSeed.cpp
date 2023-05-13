#include "HealSeed.h"

#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "ProjetHiver/Characters/Mercenary.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"

AHealSeed::AHealSeed()
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(SceneComponent);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Capsule Component"));
	SphereComponent->SetupAttachment(SceneComponent);

	bReplicates = true;
}

void AHealSeed::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AHealSeed::OnCapsuleOverlap);
}

void AHealSeed::OnCapsuleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (const APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor); IsValid(Player))
	{
		if (HasAuthority())
		{
			for (const APlayerState* PlayerState : GetWorld()->GetGameState()->PlayerArray)
				if (APlayerCharacter* OtherPlayer = PlayerState->GetPawn<APlayerCharacter>(); IsValid(OtherPlayer))
				{
					OtherPlayer->GetAbilitySystemComponent()->ApplyGameplayEffectToSelf(HealSeedEffect.GetDefaultObject(), 1, {});
					for (const AMercenary* Mercenary : OtherPlayer->GetMercenaries()) {
						Mercenary->GetAbilitySystemComponent()->ApplyGameplayEffectToSelf(HealSeedEffect.GetDefaultObject(), 1, {});
					}
				}
		}
		Destroy();
	}
}
