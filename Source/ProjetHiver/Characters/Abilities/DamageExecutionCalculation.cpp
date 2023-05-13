#include "DamageExecutionCalculation.h"

#include "ProjetHiver/Characters/BaseCharacter.h"
#include "ProjetHiver/Characters/AttributeSets/HealthAttributeSet.h"
#include "ProjetHiver/Characters/AttributeSets/PowerAttributeSet.h"

struct EFHDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalPower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalDefense);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockingPhysicalDefense);

	DECLARE_ATTRIBUTE_CAPTUREDEF(MagicalPower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(MagicalDefense);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockingMagicalDefense);

	EFHDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UPowerAttributeSet, PhysicalPower, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UPowerAttributeSet, PhysicalDefense, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UPowerAttributeSet, BlockingPhysicalDefense, Target, false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(UPowerAttributeSet, MagicalPower, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UPowerAttributeSet, MagicalDefense, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UPowerAttributeSet, BlockingMagicalDefense, Target, false);
	}
};

static const EFHDamageStatics& DamageStatics()
{
	static EFHDamageStatics DStatics;
	return DStatics;
}

UDamageExecutionCalculation::UDamageExecutionCalculation()
{
	RelevantAttributesToCapture.Add(DamageStatics().PhysicalPowerDef);
	RelevantAttributesToCapture.Add(DamageStatics().PhysicalDefenseDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockingPhysicalDefenseDef);

	RelevantAttributesToCapture.Add(DamageStatics().MagicalPowerDef);
	RelevantAttributesToCapture.Add(DamageStatics().MagicalDefenseDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockingMagicalDefenseDef);
}

void UDamageExecutionCalculation::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceAbilitySystemComponent = ExecutionParams.GetSourceAbilitySystemComponent();
	AActor* SourceActor = SourceAbilitySystemComponent ? SourceAbilitySystemComponent->GetAvatarActor() : nullptr;

	const UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();
	AActor* TargetActor = TargetAbilitySystemComponent ? TargetAbilitySystemComponent->GetAvatarActor() : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// Gather the tags from the source and target as that can affect which buffs should be used
	//const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	//EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// Get the characters' stats that will be used to compute the damage
	float PhysicalPower = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().PhysicalPowerDef, EvaluationParameters, PhysicalPower);
	PhysicalPower = FMath::Max<float>(PhysicalPower, 0.0f);

	float PhysicalDefense = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().PhysicalDefenseDef, EvaluationParameters, PhysicalDefense);
	PhysicalDefense = FMath::Max<float>(PhysicalDefense, 0.0f);

	float BlockingPhysicalDefense = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockingPhysicalDefenseDef, EvaluationParameters, BlockingPhysicalDefense);
	BlockingPhysicalDefense = FMath::Max<float>(BlockingPhysicalDefense, 0.0f);

	float MagicalPower = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().MagicalPowerDef, EvaluationParameters, MagicalPower);
	MagicalPower = FMath::Max<float>(MagicalPower, 0.0f);

	float MagicalDefense = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().MagicalDefenseDef, EvaluationParameters, MagicalDefense);
	MagicalDefense = FMath::Max<float>(MagicalDefense, 0.0f);

	float BlockingMagicalDefense = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockingMagicalDefenseDef, EvaluationParameters, BlockingMagicalDefense);
	BlockingMagicalDefense = FMath::Max<float>(BlockingMagicalDefense, 0.0f);

	// Get the SetByCaller values
	const float PhysicalDamage = FMath::Max<float>(Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("AbilityData.PhysicalDamage")), false, -1.0f), 0.0f);
	const float PhysicalPowerScaling = FMath::Max<float>(Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("AbilityData.PhysicalPowerScaling")), false, -1.0f), 0.0f);
	const float MagicalDamage = FMath::Max<float>(Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("AbilityData.MagicalDamage")), false, -1.0f), 0.0f);
	const float MagicalPowerScaling = FMath::Max<float>(Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("AbilityData.MagicalPowerScaling")), false, -1.0f), 0.0f);

	// Test if the target is a blocking character
	bool bBlocking = false;
	if (const ABaseCharacter* TargetCharacter = Cast<ABaseCharacter>(TargetActor))
		if (TargetTags->HasTagExact(FGameplayTag::RequestGameplayTag(FName("Character.Action.Move.Block"))))
			bBlocking = TargetCharacter->IsSuccessfullyBlocking(SourceActor);

	const float UnmitigatedPhysicalDamage = (PhysicalDamage + PhysicalPower * PhysicalPowerScaling / 100);
	const float UnmitigatedMagicalDamage  = (MagicalDamage  + MagicalPower  * MagicalPowerScaling  / 100);

	const float LowerRandom = (100 - DamageRandomPercentage) / 100;
	const float UpperRandom = (100 + DamageRandomPercentage) / 100;

	const float RandomizedPhysicalDamage = FMath::RandRange(UnmitigatedPhysicalDamage * LowerRandom, UnmitigatedPhysicalDamage * UpperRandom);
	const float RandomizedMagicalDamage  = FMath::RandRange(UnmitigatedMagicalDamage  * LowerRandom, UnmitigatedMagicalDamage  * UpperRandom);

	float TotalDamage;
	if (bBlocking)
		TotalDamage = RandomizedPhysicalDamage * (1 - FMath::Min<float>(PhysicalDefense + BlockingPhysicalDefense, DefensePercentageLimit) / 100) +
					  RandomizedMagicalDamage  * (1 - FMath::Min<float>(MagicalDefense  + BlockingMagicalDefense,  DefensePercentageLimit) / 100);
	else
		TotalDamage = RandomizedPhysicalDamage * (1 - FMath::Min<float>(PhysicalDefense, DefensePercentageLimit) / 100) +
					  RandomizedMagicalDamage  * (1 - FMath::Min<float>(MagicalDefense,  DefensePercentageLimit) / 100);

	TotalDamage = FMath::RoundFromZero(TotalDamage);
	TotalDamage = FMath::Max<float>(TotalDamage, MinimumDamage);

	// Set the Target's damage meta attribute
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UHealthAttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, TotalDamage));
}
