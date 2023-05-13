#include "FailableContract.h"

#include "ContractStatus.h"
#include "Net/UnrealNetwork.h"

void AFailableContract::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFailableContract, CurrentValue);
}

int AFailableContract::GetLimit() const noexcept
{
	return Limit;
}

int AFailableContract::GetCurrentValue() const noexcept
{
	return CurrentValue;
}

void AFailableContract::SetCurrentValue(const int NewValue) noexcept
{
	CurrentValue = NewValue;

	if (CurrentValue >= Limit)
		Status = EContractStatus::Failed;
}

void AFailableContract::IncreaseCurrentValue() noexcept
{
	SetCurrentValue(CurrentValue + 1);
}

EFailableContractType AFailableContract::GetType() const noexcept
{
	return Type;
}

void AFailableContract::OnRep_CurrentValueRep() const
{
	RefreshContractPanels();
}
