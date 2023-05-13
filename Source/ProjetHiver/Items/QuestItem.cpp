#include "QuestItem.h"

#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/Contracts/ContractStatus.h"
#include "ProjetHiver/Contracts/ExplorationContractType.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"


bool AQuestItem::IsInteractible(const APlayerCharacter* Player) const
{
	return Player->IsAliveTag();
}

void AQuestItem::StartInteract(APlayerCharacter* Player)
{
	if (const AEfhorisGameState* GameState = GetWorld()->GetGameState<AEfhorisGameState>(); IsValid(GameState))
		if (AExplorationContract* Contract = GameState->GetExplorationContract(); IsValid(Contract))
			if (Contract->GetType() == EExplorationContractType::FetchItem && Contract->GetStatus() == EContractStatus::OnGoing)
			{
				Contract->SetStatus(EContractStatus::Completable);
				Destroy();
			}
}

void AQuestItem::StopInteract(APlayerCharacter* Player) {}

const FText AQuestItem::GetInteractionText() const
{
	return InteractionText;
}
