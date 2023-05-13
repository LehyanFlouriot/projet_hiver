#include "EfhorisGameplayAbility.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "ProjetHiver/Characters/BaseCharacter.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"
#include "ProjetHiver/Widgets/SpellSlot.h"

void UEfhorisGameplayAbility::SetUserMovement(const EMovementMode& MovementMode) const
{
	if (const ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwningActorFromActorInfo()); IsValid(Character))
		Character->GetCharacterMovement()->SetMovementMode(MovementMode);
}

void UEfhorisGameplayAbility::SetUserToNoMoveMode() const
{
	SetUserMovement(MOVE_None);
}

void UEfhorisGameplayAbility::SetUserToWalkingMode() const
{
	SetUserMovement(MOVE_Walking);
}

void UEfhorisGameplayAbility::SetUserToFlyingMode() const
{
	SetUserMovement(MOVE_Flying);
}

void UEfhorisGameplayAbility::SetUserToFallingMode() const
{
	SetUserMovement(MOVE_Falling);
}

FGameplayEffectSpecHandle UEfhorisGameplayAbility::CreateDamageEffect(const TSubclassOf<UGameplayEffect> DamageEffect, const FDamageInfo& DamageInfo) const
{
	const FGameplayEffectSpecHandle GESpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffect, FMath::Max(1, GetAbilityLevel()));
	FGameplayEffectSpec* GESpec = GESpecHandle.Data.Get();

	const int BonusLevel = FMath::Max(0, GetAbilityLevel() - 1);

	GESpec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("AbilityData.PhysicalDamage")), DamageInfo.PhysicalDamage + DamageInfo.PhysicalDamageBonusPerLevel * BonusLevel);
	GESpec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("AbilityData.PhysicalPowerScaling")), DamageInfo.PhysicalPowerScaling);
	GESpec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("AbilityData.MagicalDamage")), DamageInfo.MagicalDamage + DamageInfo.MagicalDamageBonusPerLevel * BonusLevel);
	GESpec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("AbilityData.MagicalPowerScaling")), DamageInfo.MagicalPowerScaling);

	return GESpecHandle;
}

FGameplayEffectSpecHandle UEfhorisGameplayAbility::CreateBuffEffect(TSubclassOf<UGameplayEffect> BuffEffect,
	const float MaxHealthMultiplier, const float MaxHealthMultiplierBoostByLevel,
	const float MaxSpeedMultiplier, const float MaxSpeedMultiplierBoostByLevel,
	const float PhysicalPowerMultiplier, const float PhysicalPowerMultiplierBoostByLevel,
	const float PhysicalDefenseMultiplier, const float PhysicalDefenseMultiplierBoostByLevel,
	const float MagicalPowerMultiplier, const float MagicalPowerMultiplierBoostByLevel,
	const float MagicalDefenseMultiplier, const float MagicalDefenseMultiplierBoostByLevel) const
{
	const FGameplayEffectSpecHandle GESpecHandle = MakeOutgoingGameplayEffectSpec(BuffEffect, FMath::Max(1, GetAbilityLevel()));
	FGameplayEffectSpec* GESpec = GESpecHandle.Data.Get();

	const int BonusLevel = FMath::Max(0, GetAbilityLevel() - 1);

	GESpec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Character.Statistics.MaxHealth")), MaxHealthMultiplier + MaxHealthMultiplierBoostByLevel * BonusLevel);
	GESpec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Character.Statistics.MaxSpeed")), MaxSpeedMultiplier + MaxSpeedMultiplierBoostByLevel * BonusLevel);
	GESpec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Character.Statistics.PhysicalPower")), PhysicalPowerMultiplier + PhysicalPowerMultiplierBoostByLevel * BonusLevel);
	GESpec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Character.Statistics.PhysicalDefense")), PhysicalDefenseMultiplier + PhysicalDefenseMultiplierBoostByLevel * BonusLevel);
	GESpec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Character.Statistics.MagicalPower")), MagicalPowerMultiplier + MagicalPowerMultiplierBoostByLevel * BonusLevel);
	GESpec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Character.Statistics.MagicalDefense")), MagicalDefenseMultiplier + MagicalDefenseMultiplierBoostByLevel * BonusLevel);

	return GESpecHandle;
}

void UEfhorisGameplayAbility::InitCooldownOnWidget() const
{
	if (const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOwningActorFromActorInfo()); IsValid(PlayerCharacter))
		if (AEfhorisPlayerController* Controller = Cast<AEfhorisPlayerController>(PlayerCharacter->GetController()); IsValid(Controller))
			switch (AbilityInputID)
			{
			case EGASAbilityInputID::DodgeAction:
				Controller->Client_InitCooldownSpell(ESpellSlotType::Dodge);
				break;
			case EGASAbilityInputID::RightClickAction:
				Controller->Client_InitCooldownSpell(ESpellSlotType::RightClick);
				break;
			case EGASAbilityInputID::Skill1Action:
				Controller->Client_InitCooldownSpell(ESpellSlotType::Skill1);
				break;
			case EGASAbilityInputID::Skill2Action:
				Controller->Client_InitCooldownSpell(ESpellSlotType::Skill2);
				break;
			default:
				break;
			}
}

void UEfhorisGameplayAbility::CancelCooldownOnWidget() const
{
	if (const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOwningActorFromActorInfo()); IsValid(PlayerCharacter))
		if (AEfhorisPlayerController* Controller = Cast<AEfhorisPlayerController>(PlayerCharacter->GetController()); IsValid(Controller))
			switch (AbilityInputID)
			{
			case EGASAbilityInputID::DodgeAction:
				Controller->Client_CancelCooldownSpell(ESpellSlotType::Dodge);
				break;
			case EGASAbilityInputID::RightClickAction:
				Controller->Client_CancelCooldownSpell(ESpellSlotType::RightClick);
				break;
			case EGASAbilityInputID::Skill1Action:
				Controller->Client_CancelCooldownSpell(ESpellSlotType::Skill1);
				break;
			case EGASAbilityInputID::Skill2Action:
				Controller->Client_CancelCooldownSpell(ESpellSlotType::Skill2);
				break;
			default:
				break;
			}
}

void UEfhorisGameplayAbility::ApplyCooldownOnWidget() const
{
	if (const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOwningActorFromActorInfo()); IsValid(PlayerCharacter))
		if (AEfhorisPlayerController* Controller = Cast<AEfhorisPlayerController>(PlayerCharacter->GetController()); IsValid(Controller))
			switch (AbilityInputID)
			{
		case EGASAbilityInputID::DodgeAction:
				Controller->Client_CooldownSpell(CooldownDuration, ESpellSlotType::Dodge);
				break;
			case EGASAbilityInputID::RightClickAction:
				Controller->Client_CooldownSpell(CooldownDuration, ESpellSlotType::RightClick);
				break;
			case EGASAbilityInputID::Skill1Action:
				Controller->Client_CooldownSpell(CooldownDuration, ESpellSlotType::Skill1);
				break;
			case EGASAbilityInputID::Skill2Action:
				Controller->Client_CooldownSpell(CooldownDuration, ESpellSlotType::Skill2);
				break;
			default:
				break;
			}
}

EGASAbilityInputID UEfhorisGameplayAbility::GetAbilityInputID() const
{
	return AbilityInputID;
}

void UEfhorisGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (CooldownGE)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
		SpecHandle.Data.Get()->DynamicGrantedTags.AppendTags(CooldownTags);
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("AbilityData.Cooldown")), CooldownDuration);
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		ApplyCooldownOnWidget();
	}
}

const FGameplayTagContainer* UEfhorisGameplayAbility::GetCooldownTags() const
{
	FGameplayTagContainer* MutableTags = const_cast<FGameplayTagContainer*>(&TempCooldownTags);
	MutableTags->Reset(); // MutableTags writes to the TempCooldownTags on the CDO so clear it in case the ability cooldown tags change (moved to a different slot)
	const FGameplayTagContainer* ParentTags = Super::GetCooldownTags();
	if (ParentTags)
	{
		MutableTags->AppendTags(*ParentTags);
	}
	MutableTags->AppendTags(CooldownTags);
	return MutableTags;
}
