#include "PlayersStatsMeasuringComponent.h"

#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/MapGeneration/TileLimit.h"

// Sets default values for this component's properties
UPlayersStatsMeasuringComponent::UPlayersStatsMeasuringComponent()
{
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);
}

void UPlayersStatsMeasuringComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (APlayerCharacter* Owner = Cast<APlayerCharacter>(GetOwner()); ensure(IsValid(Owner)))
		Owner->OnActorEndOverlap.AddDynamic(this, &UPlayersStatsMeasuringComponent::OnOwnerEndOverlap);
}

void UPlayersStatsMeasuringComponent::UninitializeComponent()
{
	Super::UninitializeComponent();

	if (APlayerCharacter* Owner = Cast<APlayerCharacter>(GetOwner()); IsValid(Owner))
		Owner->OnActorEndOverlap.RemoveDynamic(this, &UPlayersStatsMeasuringComponent::OnOwnerEndOverlap);
}

void UPlayersStatsMeasuringComponent::OnOwnerEndOverlap(AActor* OwningActor, AActor* OtherActor)
{
	if (OwningActor->HasAuthority())
	{
		if (const APlayerCharacter* Owner = Cast<APlayerCharacter>(OwningActor); IsValid(Owner)) {
			if (OtherActor->IsA<ATileLimit>())
			{
				ATileLimit* Limit = Cast<ATileLimit>(OtherActor);
				if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
				{
					if (AEfhorisGameMode* GameMode = Cast<AEfhorisGameMode>(World->GetAuthGameMode()); ensure(IsValid(GameMode)))
					{
						const FVector Position = Owner->GetActorLocation();

						GameMode->TileChanged(Limit, FVector2D(Position.X, Position.Y));
					}
				}
			}
		}
	}
}

void UPlayersStatsMeasuringComponent::Authority_AddDamageTaken(double Damage) const
{
	if (const APlayerCharacter* Owner = Cast<APlayerCharacter>(GetOwner()); IsValid(Owner))
	{
		if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
		{
			if (const AEfhorisGameMode* GameMode = Cast<AEfhorisGameMode>(World->GetAuthGameMode()); ensure(IsValid(GameMode)))
			{
				GameMode->AddDamageTaken(Damage);
			}
		}
	}
}
