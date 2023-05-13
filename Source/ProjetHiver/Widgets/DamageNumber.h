// Copyright 2020 Dan Kestranek.
#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "DamageNumber.generated.h"

UCLASS(Blueprintable)
class PROJETHIVER_API UDamageNumber final : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UDamageNumber();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetDamageText(float Damage);

};
