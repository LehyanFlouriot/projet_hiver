#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Widgets/Panels/EfhorisPanel.h"
#include "SpellsPanel.generated.h"

class USpellSlot;

UCLASS()
class PROJETHIVER_API USpellsPanel : public UUserWidget
{
	GENERATED_BODY()
protected:
	UPROPERTY(BlueprintReadWrite, Transient)
	USpellSlot* SlotDodge;

	UPROPERTY(BlueprintReadWrite, Transient)
	USpellSlot* SlotRightClick;

	UPROPERTY(BlueprintReadWrite, Transient)
	USpellSlot* SlotSkill1;

	UPROPERTY(BlueprintReadWrite, Transient)
	USpellSlot* SlotSkill2;
public:
	UFUNCTION(BlueprintCallable)
	void RefreshPanel();

	UFUNCTION()
	FORCEINLINE	USpellSlot* GetDodgeSlot() const noexcept{ return SlotDodge; };
	UFUNCTION()
	FORCEINLINE	USpellSlot* GetSlotRightClick() const noexcept { return SlotRightClick; };
	UFUNCTION()
	FORCEINLINE	USpellSlot* GetSlotSkill1() const noexcept  { return SlotSkill1; };
	UFUNCTION()
	FORCEINLINE	USpellSlot* GetSlotSkill2() const noexcept  { return SlotSkill2; };
	
};
