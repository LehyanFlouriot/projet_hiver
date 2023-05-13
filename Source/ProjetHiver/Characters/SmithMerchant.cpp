#include "SmithMerchant.h"

#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/Contracts/ContractStatus.h"
#include "ProjetHiver/Contracts/ExplorationContractType.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"

bool ASmithMerchant::IsInteractible(const APlayerCharacter* Player) const
{
	return Player->IsAliveTag();
}

void ASmithMerchant::StartInteract(APlayerCharacter* Player)
{
	if (const AEfhorisGameState* GameState = GetWorld()->GetGameState<AEfhorisGameState>(); IsValid(GameState))
		if (AExplorationContract* Contract = GameState->GetExplorationContract(); IsValid(Contract))
			if (Contract->GetType() == EExplorationContractType::FetchItem && Contract->GetStatus() == EContractStatus::Completable)
			{
				Contract->SetStatus(EContractStatus::Succeeded);
				return;
			}
	if (AEfhorisPlayerController* PlayerController = Player->GetController<AEfhorisPlayerController>(); ensure(IsValid(PlayerController)))
	{
		PlayerController->Client_OpenInventoryPanel();
		PlayerController->Client_OpenSmithPanel();
	}
}

void ASmithMerchant::StopInteract(APlayerCharacter* Player)
{
	if (AEfhorisPlayerController* PlayerController = Player->GetController<AEfhorisPlayerController>(); ensure(IsValid(PlayerController)))
	{
		PlayerController->Client_CloseInventoryPanel();
		PlayerController->Client_CloseSmithPanel();
		if(APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(PlayerController->GetCharacter()); ensure(IsValid(PlayerCharacter)))
			if (AEfhorisGameMode* EfhorisGameMode = GetWorld()->GetAuthGameMode<AEfhorisGameMode>(); ensure(IsValid(EfhorisGameMode)))
				EfhorisGameMode->RemoveSmithItemFromPlayer(PlayerCharacter);
	}
}

const FText ASmithMerchant::GetInteractionText() const
{
	if (const AEfhorisGameState* GameState = GetWorld()->GetGameState<AEfhorisGameState>(); IsValid(GameState))
		if (AExplorationContract* Contract = GameState->GetExplorationContract(); IsValid(Contract))
			if (Contract->GetType() == EExplorationContractType::FetchItem && Contract->GetStatus() == EContractStatus::Completable)
				return BringBackHammerText;
	return InteractionText;
}
