#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SpellSlot.generated.h"

UENUM(BlueprintType)
enum class ESpellSlotType : uint8
{
	Dodge UMETA(DisplayName = "Dodge"),
	Skill1 UMETA(DisplayName = "Skill1"),
	Skill2 UMETA(DisplayName = "Skill2"),
	RightClick UMETA(DisplayName = "RightClick"),
	Num UMETA(Hidden)
};

UCLASS()
class PROJETHIVER_API USpellSlot : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ESpellSlotType SpellType;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void RefreshSlot();

	UFUNCTION(BlueprintImplementableEvent)
	void CastSpell(float Cooldown);

	UFUNCTION(BlueprintImplementableEvent)
	void InitCooldown();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void CancelCooldown();
};
