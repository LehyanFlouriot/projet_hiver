#include "ProjetHiver/Widgets/SpellsPanel.h"

#include "ProjetHiver/Widgets/SpellSlot.h"

void USpellsPanel::RefreshPanel()
{
	if (IsValid(SlotDodge))
		SlotDodge->RefreshSlot();
	if(IsValid(SlotRightClick))
		SlotRightClick->RefreshSlot();
	if(IsValid(SlotSkill1))
		SlotSkill1->RefreshSlot();
	if(IsValid(SlotSkill2))
		SlotSkill2->RefreshSlot();
}
