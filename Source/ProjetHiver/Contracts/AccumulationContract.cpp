#include "AccumulationContract.h"
#include "AccumulationContractType.h"
#include "ContractStatus.h"
#include "Net/UnrealNetwork.h"
#include "ProjetHiver/Widgets/GameView.h"

void AAccumulationContract::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAccumulationContract, Count);
}

int AAccumulationContract::GetCount() const noexcept
{
	return Count;
}

void AAccumulationContract::SetCount(const int NewCount) noexcept
{
	Count = NewCount;
}

void AAccumulationContract::IncreaseCount() noexcept
{
	if (GetStatus() != EContractStatus::OnGoing) return;

	Count++;
}

EAccumulationContractType AAccumulationContract::GetType() const noexcept
{
	return Type;
}

void AAccumulationContract::OnRep_CountRep() const
{
	RefreshContractPanels();
}
