#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/GAS/EfhorisAttributeSet.h"
#include "PotionAttributeSet.generated.h"

UCLASS()
class PROJETHIVER_API UPotionAttributeSet : public UEfhorisAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion", ReplicatedUsing = OnRep_PotionCharge)
	FGameplayAttributeData PotionCharge;
	ATTRIBUTE_ACCESSORS(UPotionAttributeSet, PotionCharge);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion", ReplicatedUsing = OnRep_MaxPotionCharge)
	FGameplayAttributeData MaxPotionCharge;
	ATTRIBUTE_ACCESSORS(UPotionAttributeSet, MaxPotionCharge);

protected:
	UFUNCTION()
	void OnRep_PotionCharge(const FGameplayAttributeData& OldPotionCharge) const;

	UFUNCTION()
	void OnRep_MaxPotionCharge(const FGameplayAttributeData& OldMaxPotionCharge) const;
};
