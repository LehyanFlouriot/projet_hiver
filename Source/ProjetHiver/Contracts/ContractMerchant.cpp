#include "ContractMerchant.h"

#include "Engine/DataTable.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"


bool AContractMerchant::IsInteractible(const APlayerCharacter* Player) const {
	return Player->IsAliveTag();
}

void AContractMerchant::StartInteract(APlayerCharacter* Player)
{
	if (AEfhorisPlayerController* PlayerController = Player->GetController<AEfhorisPlayerController>(); ensure(IsValid(PlayerController)))
		PlayerController->Client_OpenContractPanels();
}

void AContractMerchant::StopInteract(APlayerCharacter* Player)
{
	if (AEfhorisPlayerController* PlayerController = Player->GetController<AEfhorisPlayerController>(); ensure(IsValid(PlayerController)))
		PlayerController->Client_CloseContractPanels();
}

const FText AContractMerchant::GetInteractionText() const
{
	return InteractionText;
}
