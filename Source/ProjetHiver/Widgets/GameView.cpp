#include "ProjetHiver/Widgets/GameView.h"

#include "SpellSlot.h"
#include "Panels/Minimap.h"
#include "ProjetHiver/Widgets/Panels/MedicPanel.h"
#include "ProjetHiver/Widgets/Panels/BoxHealthTeamPanel.h"
#include "ProjetHiver/Widgets/Panels/EfhorisPanel.h"
#include "ProjetHiver/Widgets/SpellsPanel.h"
#include "ProjetHiver/Widgets/HealthPotion.h"
#include "ProjetHiver/Widgets/FullscreenWidget.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"

void UGameView::RefreshInventoryPanel()
{
	InventoryPanel->RefreshPanel();
}

void UGameView::RefreshEquipmentPanel()
{
	EquipmentPanel->RefreshPanel();
}

void UGameView::RefreshShopPanel()
{
	ShopPanel->RefreshPanel();
}

void UGameView::RefreshSmithPanel()
{
	SmithPanel->RefreshPanel();
}

void UGameView::RefreshMedicPanel()
{
	MedicPanel->RefreshPanel();
}

void UGameView::RefreshBoxHealthTeamPanel(const TArray<FPlayerInfo>& Descriptions)
{
	BoxHealthTeamPanel->RefreshHealthTeamPanel(Descriptions);
	MedicPanel->RefreshHealthTeamPanel(Descriptions);
}

void UGameView::RefreshMapNicknames(const TArray<FPlayerInfo>& Descriptions) {
	Minimap->RefreshNicknames(Descriptions);
}

void UGameView::RefreshHealthPotion()
{
	HealthPotion->RefreshHealthPotion();
}

void UGameView::RefreshAccumulationContractPanel()
{
	AccumulationContractPanel->RefreshPanel();
}

void UGameView::RefreshExplorationContractPanel()
{
	ExplorationContractPanel->RefreshPanel();
}

void UGameView::RefreshFailableContractPanel()
{
	FailableContractPanel->RefreshPanel();
}

void UGameView::RefreshContractPanels()
{
	RefreshAccumulationContractPanel();
	RefreshExplorationContractPanel();
	RefreshFailableContractPanel();
}

void UGameView::RefreshSideContractPanel()
{
	SideContractPanel->RefreshPanel();
}
void UGameView::RefreshSpellsPanel()
{
	SpellsPanel->RefreshPanel();
}

void UGameView::Win() {
	VictoryScreen->SetVisibility(ESlateVisibility::Visible);
	VictoryScreen->Show();
}

void UGameView::Lose() {
	DefeatScreen->SetVisibility(ESlateVisibility::Visible);
	DefeatScreen->Show();
}
USpellSlot* UGameView::GetSpell(ESpellSlotType SpellType) const noexcept
{
	switch (SpellType)
	{
	case ESpellSlotType::Dodge: return SpellsPanel->GetDodgeSlot();
	case ESpellSlotType::RightClick: return SpellsPanel->GetSlotRightClick();
	case ESpellSlotType::Skill1: return SpellsPanel->GetSlotSkill1();
	case ESpellSlotType::Skill2: return SpellsPanel->GetSlotSkill2();
	default: return nullptr;
	}
}
