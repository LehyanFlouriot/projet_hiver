#include "Contract.h"
#include "ContractStatus.h"
#include "Net/UnrealNetwork.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/EfhorisCore/EfhorisHUD.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"
#include "ProjetHiver/Widgets/GameView.h"

AContract::AContract()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AContract::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AContract, Status);
}

FText AContract::GetTitle() const noexcept
{
	return Title;
}

FText AContract::GetSummary() const noexcept
{
	return Summary;
}

FText AContract::GetDescription() const noexcept
{
	return Description;
}

FText AContract::GetHUDSummary() const noexcept
{
	return HUDSummary;
}

int AContract::GetCost() const noexcept
{
	return Cost;
}

int AContract::GetReward() const noexcept
{
	return Reward;
}

EContractStatus AContract::GetStatus() const noexcept
{
	return Status;
}

void AContract::SetStatus(const EContractStatus NewStatus) noexcept
{
	Status = NewStatus;
}

void AContract::OnRep_StatusRep() const
{
	RefreshContractPanels(true);
}

void AContract::RefreshContractPanels(const bool bRefreshOpenedPanels) const
{
	if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
		if (AEfhorisPlayerController* PlayerController = CastChecked<AEfhorisPlayerController>(World->GetFirstPlayerController()); ensure(IsValid(PlayerController)))
		{
			if (bRefreshOpenedPanels)
				PlayerController->Client_RefreshOpenedPanels();

			if (const AEfhorisHUD* HUD = PlayerController->GetHUD<AEfhorisHUD>(); IsValid(HUD))
				if (UGameView* GameView = HUD->GetGameView(); IsValid(GameView))
					GameView->RefreshSideContractPanel();
		}
}
