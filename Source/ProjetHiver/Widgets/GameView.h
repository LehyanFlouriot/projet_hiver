#pragma once

#include "CoreMinimal.h"
#include "SpellsPanel.h"
#include "Blueprint/UserWidget.h"
#include "GameView.generated.h"

class UBoxHealthTeamPanel;
class UEfhorisPanel;
class UToggleableMenu;
class UInteractionPrompt;
class UHealthBoss;
class UHealthPlayer;
class UHealthPotion;
class UMedicPanel;
class USpellsPanel;
class USpellSlot;
struct FPlayerInfo;
class UFullscreenWidget;
enum class ESpellSlotType : uint8;
class UIngameMainMenu;
class UMinimap;

UCLASS()
class PROJETHIVER_API UGameView : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, Transient)
	UEfhorisPanel* InventoryPanel;

	UPROPERTY(BlueprintReadWrite, Transient)
	UEfhorisPanel* EquipmentPanel;

	UPROPERTY(BlueprintReadWrite, Transient)
	UEfhorisPanel* ShopPanel;

	UPROPERTY(BlueprintReadWrite, Transient)
	UEfhorisPanel* SmithPanel;

	UPROPERTY(BlueprintReadWrite, Transient)
	UMedicPanel* MedicPanel;

	UPROPERTY(BlueprintReadWrite, Transient)
	UBoxHealthTeamPanel* BoxHealthTeamPanel;

	UPROPERTY(BlueprintReadWrite, Transient)
	UInteractionPrompt* InteractPrompt;

	UPROPERTY(BlueprintReadWrite, Transient)
	UHealthPlayer* HealthBar;

	UPROPERTY(BlueprintReadWrite, Transient)
	UHealthBoss* BossHealthBar;

	UPROPERTY(BlueprintReadWrite, Transient)
	UHealthPotion* HealthPotion;

	UPROPERTY(BlueprintReadWrite, Transient)
	UWidget* EmplacementDetailsInventory;

	UPROPERTY(BlueprintReadWrite, Transient)
	UWidget* EmplacementDetailsShop;

	UPROPERTY(BlueprintReadWrite, Transient)
	UWidget* EmplacementDetailsSmith;

	UPROPERTY(BlueprintReadWrite, Transient)
	UEfhorisPanel* AccumulationContractPanel;

	UPROPERTY(BlueprintReadWrite, Transient)
	UEfhorisPanel* ExplorationContractPanel;

	UPROPERTY(BlueprintReadWrite, Transient)
	UEfhorisPanel* FailableContractPanel;

	UPROPERTY(BlueprintReadWrite, Transient)
	UEfhorisPanel* SideContractPanel;

	UPROPERTY(BlueprintReadWrite, Transient)
	USpellsPanel* SpellsPanel;

	UPROPERTY(BlueprintReadWrite, Transient)
	UFullscreenWidget* VictoryScreen;

	UPROPERTY(BlueprintReadWrite, Transient)
	UFullscreenWidget* DefeatScreen;

	UPROPERTY(BlueprintReadWrite, Transient)
	UFullscreenWidget* LoadingScreen;

	UPROPERTY(BlueprintReadWrite, Transient)
	UIngameMainMenu* IngameMenu;

	UPROPERTY(BlueprintReadWrite, Transient)
	UMinimap* Minimap;

	UPROPERTY(BlueprintReadWrite, Transient)
	UUserWidget* CrosshairWidget;

	UPROPERTY(BlueprintReadWrite, Transient)
	UUserWidget* ChestMoving;

public:
	void RefreshInventoryPanel();
	void RefreshEquipmentPanel();
	void RefreshShopPanel();
	void RefreshSmithPanel();
	void RefreshMedicPanel();
	void RefreshBoxHealthTeamPanel(const TArray<FPlayerInfo>& Descriptions);
	void RefreshMapNicknames(const TArray<FPlayerInfo>& Descriptions);
	void RefreshHealthPotion();
	void RefreshAccumulationContractPanel();
	void RefreshExplorationContractPanel();
	void RefreshFailableContractPanel();
	void RefreshContractPanels();
	void RefreshSideContractPanel();
	void Win();
	void Lose();
	void RefreshSpellsPanel();
	USpellSlot* GetSpell(ESpellSlotType SpellType) const noexcept;

	FORCEINLINE UEfhorisPanel* GetInventoryPanel() const noexcept { return InventoryPanel; }
	FORCEINLINE UEfhorisPanel* GetEquipmentPanel() const noexcept { return EquipmentPanel; }
	FORCEINLINE UEfhorisPanel* GetShopPanel() const noexcept { return ShopPanel; }
	FORCEINLINE UEfhorisPanel* GetSmithPanel() const noexcept { return SmithPanel; }
	FORCEINLINE UMedicPanel* GetMedicPanel() const noexcept { return MedicPanel; }
	FORCEINLINE UBoxHealthTeamPanel* GetBoxHealthTeamPanel() const noexcept { return BoxHealthTeamPanel; }
	FORCEINLINE UInteractionPrompt* GetInteractPrompt() const noexcept { return InteractPrompt; }
	FORCEINLINE UHealthPlayer* GetHealthBar() const noexcept { return HealthBar; }
	FORCEINLINE UHealthBoss* GetBossHealthBar() const noexcept { return BossHealthBar; }
	FORCEINLINE UEfhorisPanel* GetAccumulationContractPanel() const noexcept { return AccumulationContractPanel; }
	FORCEINLINE UEfhorisPanel* GetExplorationContractPanel() const noexcept { return ExplorationContractPanel; }
	FORCEINLINE UEfhorisPanel* GetFailableContractPanel() const noexcept { return FailableContractPanel; }
	FORCEINLINE UEfhorisPanel* GetSideContractPanel() const noexcept { return SideContractPanel; }
	FORCEINLINE USpellsPanel* GetSpellsPanel() const noexcept { return SpellsPanel; }
	FORCEINLINE USpellSlot* GetRollSlot() const noexcept { return SpellsPanel->GetDodgeSlot(); }
	FORCEINLINE UIngameMainMenu* GetIngameMenu() const noexcept { return  IngameMenu; }
	FORCEINLINE UFullscreenWidget* GetLoadingScreen() const noexcept { return  LoadingScreen; }
	FORCEINLINE UMinimap* GetMinimap() const noexcept { return  Minimap; }
	FORCEINLINE UUserWidget* GetCrosshair() const noexcept { return  CrosshairWidget; }
	FORCEINLINE UUserWidget* GetChestMoving() const noexcept { return  ChestMoving; }
};
