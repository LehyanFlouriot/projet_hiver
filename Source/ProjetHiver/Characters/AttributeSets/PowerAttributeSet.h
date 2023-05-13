#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/GAS/EfhorisAttributeSet.h"
#include "PowerAttributeSet.generated.h"

UCLASS()
class PROJETHIVER_API UPowerAttributeSet : public UEfhorisAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power", ReplicatedUsing = OnRep_PhysicalPower)
	FGameplayAttributeData PhysicalPower;
	ATTRIBUTE_ACCESSORS(UPowerAttributeSet, PhysicalPower);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power", ReplicatedUsing = OnRep_PhysicalDefense)
	FGameplayAttributeData PhysicalDefense;
	ATTRIBUTE_ACCESSORS(UPowerAttributeSet, PhysicalDefense);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power", ReplicatedUsing = OnRep_BlockingPhysicalDefense)
	FGameplayAttributeData BlockingPhysicalDefense;
	ATTRIBUTE_ACCESSORS(UPowerAttributeSet, BlockingPhysicalDefense);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power", ReplicatedUsing = OnRep_MagicalPower)
	FGameplayAttributeData MagicalPower;
	ATTRIBUTE_ACCESSORS(UPowerAttributeSet, MagicalPower);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power", ReplicatedUsing = OnRep_MagicalDefense)
	FGameplayAttributeData MagicalDefense;
	ATTRIBUTE_ACCESSORS(UPowerAttributeSet, MagicalDefense);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power", ReplicatedUsing = OnRep_BlockingMagicalDefense)
	FGameplayAttributeData BlockingMagicalDefense;
	ATTRIBUTE_ACCESSORS(UPowerAttributeSet, BlockingMagicalDefense);

protected:
	UFUNCTION()
	void OnRep_PhysicalPower(const FGameplayAttributeData& OldPhysicalPower) const;

	UFUNCTION()
	void OnRep_PhysicalDefense(const FGameplayAttributeData& OldPhysicalDefense) const;

	UFUNCTION()
	void OnRep_BlockingPhysicalDefense(const FGameplayAttributeData& OldBlockingPhysicalDefense) const;

	UFUNCTION()
	void OnRep_MagicalPower(const FGameplayAttributeData& OldMagicalPower) const;

	UFUNCTION()
	void OnRep_MagicalDefense(const FGameplayAttributeData& OldMagicalDefense) const;

	UFUNCTION()
	void OnRep_BlockingMagicalDefense(const FGameplayAttributeData& OldBlockingMagicalDefense) const;
};
