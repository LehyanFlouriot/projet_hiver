// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Characters/Abilities/Players/EGA_RangeBase.h"
#include "EGA_MeleeLightDistanceAttack.generated.h"

UCLASS()
class PROJETHIVER_API UEGA_MeleeLightDistanceAttack : public UEGA_RangeBase
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	void OnGameplayEventReceived(FGameplayEventData GameplayEventData) override;
};
