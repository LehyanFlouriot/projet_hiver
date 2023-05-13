#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/GAS/EfhorisAttributeSet.h"
#include "MovementAttributeSet.generated.h"

UCLASS()
class PROJETHIVER_API UMovementAttributeSet final : public UEfhorisAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speed", ReplicatedUsing = OnRep_MaxSpeed)
	FGameplayAttributeData MaxSpeed;
	ATTRIBUTE_ACCESSORS(UMovementAttributeSet, MaxSpeed);

protected:
	UFUNCTION()
	void OnRep_MaxSpeed(const FGameplayAttributeData& OldMaxSpeed) const;
};
