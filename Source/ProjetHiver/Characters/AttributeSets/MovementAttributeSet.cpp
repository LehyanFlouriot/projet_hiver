#include "MovementAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "ProjetHiver/Characters/BaseCharacter.h"

void UMovementAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMovementAttributeSet, MaxSpeed, COND_None, REPNOTIFY_Always);
}

void UMovementAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxSpeedAttribute())
	{
		// Cannot slow less than 150 units/s and cannot boost more than 1000 units/s
		NewValue = FMath::Clamp<float>(NewValue, 150, 1000);
	}
}

void UMovementAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxSpeedAttribute())
	{
		if (const ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwningActor()))
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = NewValue;
		}
	}
}

void UMovementAttributeSet::OnRep_MaxSpeed(const FGameplayAttributeData& OldMaxSpeed) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMovementAttributeSet, MaxSpeed, OldMaxSpeed);
	if (const ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwningActor()))
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = Character->GetMaxSpeed();
	}
}
