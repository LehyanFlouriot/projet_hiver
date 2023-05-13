#include "MedicPanel.h"

#include "BoxHealthTeamPanel.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"

void UMedicPanel::RefreshHealthTeamPanel(const TArray<FPlayerInfo>& Descriptions)
{
	MedicBoxHealthTeamPanel->RefreshHealthTeamPanel(Descriptions);
}
