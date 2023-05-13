#include "PotionAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

void UPotionAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UPotionAttributeSet, PotionCharge, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPotionAttributeSet, MaxPotionCharge, COND_None, REPNOTIFY_Always);
}

void UPotionAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxPotionChargeAttribute())
	{
		AdjustAttributeForMaxChange(PotionCharge, MaxPotionCharge, NewValue, GetPotionChargeAttribute());
	}
}

void UPotionAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetPotionChargeAttribute())
	{
		SetPotionCharge(FMath::Clamp(GetPotionCharge(), 0.f, GetMaxPotionCharge()));
	}
}

void UPotionAttributeSet::OnRep_PotionCharge(const FGameplayAttributeData& OldPotionCharge) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPotionAttributeSet, PotionCharge, OldPotionCharge);
}

void UPotionAttributeSet::OnRep_MaxPotionCharge(const FGameplayAttributeData& OldMaxPotionCharge) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPotionAttributeSet, MaxPotionCharge, OldMaxPotionCharge);
}
