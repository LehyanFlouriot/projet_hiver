#include "PowerAttributeSet.h"
#include "Net/UnrealNetwork.h"

void UPowerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UPowerAttributeSet, PhysicalPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPowerAttributeSet, PhysicalDefense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPowerAttributeSet, BlockingPhysicalDefense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPowerAttributeSet, MagicalPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPowerAttributeSet, MagicalDefense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPowerAttributeSet, BlockingMagicalDefense, COND_None, REPNOTIFY_Always);
}

void UPowerAttributeSet::OnRep_PhysicalPower(const FGameplayAttributeData& OldPhysicalPower) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPowerAttributeSet, PhysicalPower, OldPhysicalPower);
}

void UPowerAttributeSet::OnRep_PhysicalDefense(const FGameplayAttributeData& OldPhysicalDefense) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPowerAttributeSet, PhysicalDefense, OldPhysicalDefense);
}

void UPowerAttributeSet::OnRep_BlockingPhysicalDefense(const FGameplayAttributeData& OldBlockingPhysicalDefense) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPowerAttributeSet, BlockingPhysicalDefense, OldBlockingPhysicalDefense);
}

void UPowerAttributeSet::OnRep_MagicalPower(const FGameplayAttributeData& OldMagicalPower) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPowerAttributeSet, MagicalPower, OldMagicalPower);
}

void UPowerAttributeSet::OnRep_MagicalDefense(const FGameplayAttributeData& OldMagicalDefense) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPowerAttributeSet, MagicalDefense, OldMagicalDefense);
}

void UPowerAttributeSet::OnRep_BlockingMagicalDefense(const FGameplayAttributeData& OldBlockingMagicalDefense) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPowerAttributeSet, BlockingMagicalDefense, OldBlockingMagicalDefense);
}
