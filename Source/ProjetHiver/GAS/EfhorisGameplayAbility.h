#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ProjetHiver/ProjetHiver.h"
#include "ProjetHiver/Characters/Abilities/Structs/DamageInfo.h"
#include "EfhorisGameplayAbility.generated.h"

enum EMovementMode;

UCLASS()
class PROJETHIVER_API UEfhorisGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	// Abilities with this set will automatically activate when the input is pressed
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	EGASAbilityInputID AbilityInputID = EGASAbilityInputID::None;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	FName DisplayName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (DisplayName = "Ability|Icon", MakeStructureDefaultValue = "None"))
	TObjectPtr<UTexture2D> AbilityIcon;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability|Cooldown")
	float CooldownDuration;


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability|Cooldown")
	FGameplayTagContainer CooldownTags;

	// Temp container that we will return the pointer to in GetCooldownTags().
	// This will be a union of our CooldownTags and the Cooldown GE's cooldown tags.
	UPROPERTY(Transient)
	FGameplayTagContainer TempCooldownTags;

private:
	UFUNCTION()
	void SetUserMovement(const EMovementMode& MovementMode) const;

protected:
	UFUNCTION()
	void SetUserToNoMoveMode() const;

	UFUNCTION()
	void SetUserToWalkingMode() const;

	UFUNCTION()
	void SetUserToFlyingMode() const;

	UFUNCTION()
	void SetUserToFallingMode() const;

	UFUNCTION()
	FGameplayEffectSpecHandle CreateDamageEffect(TSubclassOf<UGameplayEffect> DamageEffect, const FDamageInfo& DamageInfo) const;

	UFUNCTION()
	FGameplayEffectSpecHandle CreateBuffEffect(TSubclassOf<UGameplayEffect> BuffEffect, 
		const float MaxHealthMultiplier, const float MaxHealthMultiplierBoostByLevel,
		const float MaxSpeedMultiplier, const float MaxSpeedMultiplierBoostByLevel,
		const float PhysicalPowerMultiplier, const float PhysicalPowerMultiplierBoostByLevel,
		const float PhysicalDefenseMultiplier, const float PhysicalDefenseMultiplierBoostByLevel,
		const float MagicalPowerMultiplier, const float MagicalPowerMultiplierBoostByLevel,
		const float MagicalDefenseMultiplier, const float MagicalDefenseMultiplierBoostByLevel) const;

protected:
	void InitCooldownOnWidget() const;
	void CancelCooldownOnWidget() const;
	void ApplyCooldownOnWidget() const;
public:
	EGASAbilityInputID GetAbilityInputID() const;

	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual const FGameplayTagContainer* GetCooldownTags() const override;
};
