#include "MedicMerchant.h"

#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"

bool AMedicMerchant::IsInteractible(const APlayerCharacter* Player) const
{
	return Player->IsAliveTag();
}

void AMedicMerchant::StartInteract(APlayerCharacter* Player)
{
	if (AEfhorisPlayerController* PlayerController = Player->GetController<AEfhorisPlayerController>(); ensure(IsValid(PlayerController)))
		PlayerController->Client_OpenMedicPanel();
}

void AMedicMerchant::StopInteract(APlayerCharacter* Player)
{
	if (AEfhorisPlayerController* PlayerController = Player->GetController<AEfhorisPlayerController>(); ensure(IsValid(PlayerController)))
		PlayerController->Client_CloseMedicPanel();
}

const FText AMedicMerchant::GetInteractionText() const
{
	return InteractionText;
}
