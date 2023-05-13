#include "PickUpComponent.h"

#include "ProjetHiver/BDD/UserScoreboardStatistic.h"
#include "ProjetHiver/Characters/BaseCharacter.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerState.h"
#include "ProjetHiver/Items/PickableItem.h"

UPickUpComponent::UPickUpComponent()
{
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);
}

void UPickUpComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if (ABaseCharacter* Owner = Cast<ABaseCharacter>(GetOwner()); ensure(IsValid(Owner)))
		Owner->OnActorBeginOverlap.AddDynamic(this, &UPickUpComponent::OnOwnerOverlap);
}

void UPickUpComponent::UninitializeComponent()
{
	Super::UninitializeComponent();
	if (ABaseCharacter* Owner = Cast<ABaseCharacter>(GetOwner()); IsValid(Owner))
		Owner->OnActorBeginOverlap.RemoveDynamic(this, &UPickUpComponent::OnOwnerOverlap);
}

void UPickUpComponent::OnOwnerOverlap(AActor* OwningActor, AActor* OtherActor)
{
	if (OwningActor->HasAuthority())
	{
		if (const ABaseCharacter* Player = Cast<ABaseCharacter>(OwningActor); IsValid(Player))
			if (!Player->IsAliveHealth()) return;

		if (APickableItem* Item = Cast<APickableItem>(OtherActor); IsValid(Item))
		{
			if (!Item->IsPickable()) return;
			if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
			{
				if (AEfhorisGameMode* GameMode = Cast<AEfhorisGameMode>(World->GetAuthGameMode()); ensure(IsValid(GameMode)))
				{
					GameMode->StoreItem(Item);
				}

				if (const APlayerCharacter* Player = Cast<APlayerCharacter>(OwningActor); IsValid(Player))
					if (AEfhorisPlayerState* PlayerState = Player->GetPlayerState<AEfhorisPlayerState>(); IsValid(PlayerState))
						PlayerState->IncreaseStatistic(EUserScoreboardStatistic::ItemsPickedUp, 1);
			}
		}
	}
}
