// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EGASAbilityInputID : uint8
{
	None,
	Confirm,
	Cancel,
	JumpAction,
	DodgeAction,
	PotionAction,
	RightClickAction,
	Skill1Action,
	Skill2Action,
	AttackComboFirstAction
};
