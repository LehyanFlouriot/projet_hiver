#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"

#include "GameplayEffect.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "AkComponent.h"
#include "ProjetHiver/AI/AIManager.h"
#include "ProjetHiver/Characters/BaseCharacter.h"
#include "ProjetHiver/Characters/BaseNPC.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/Contracts/Contract.h"
#include "ProjetHiver/Contracts/ContractStatus.h"
#include "ProjetHiver/EfhorisCore/EfhorisCheatManager.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameInstance.h"
#include "ProjetHiver/EfhorisCore/EfhorisHUD.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerState.h"
#include "ProjetHiver/Save/SaveSubsystem.h"
#include "ProjetHiver/Widgets/Panels/ContractPanel.h"
#include "ProjetHiver/Widgets/DamageNumber.h"
#include "ProjetHiver/Widgets/FloatingHealthBar.h"
#include "ProjetHiver/Widgets/GameView.h"
#include "ProjetHiver/Widgets/HealthPlayer.h"
#include "ProjetHiver/Widgets/InteractionPrompt.h"
#include "ProjetHiver/Widgets/Panels/ShopPanel.h"
#include "ProjetHiver/Widgets/Panels/MedicPanel.h"
#include "ProjetHiver/Widgets/Panels/Options.h"
#include "ProjetHiver/Widgets/Panels/IngameMainMenu.h"
#include "ProjetHiver/Interaction/InteractionComponent.h"
#include "ProjetHiver/Widgets/SpellSlot.h"
#include "ProjetHiver/Widgets/Panels/Minimap.h"
#include "LevelSequence/Public/LevelSequence.h"
#include "LevelSequence/Public/LevelSequencePlayer.h"

void AEfhorisPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AEfhorisPlayerController, MenuCooldown);
	DOREPLIFETIME(AEfhorisPlayerController, MusicTargets);
}

void AEfhorisPlayerController::OnRep_MusicTargets()
{
	if(MusicTargets.IsEmpty())
	{
		Client_SetMusicState(FName("PalierX"), FName("Hors_Combat"));
	}
	else
	{
		Client_SetMusicState(FName("PalierX"), FName("En_Combat"));
	}
}

AEfhorisPlayerController::AEfhorisPlayerController()
{
	CheatClass = UEfhorisCheatManager::StaticClass();
}

void AEfhorisPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (HasAuthority()) return;

	if (const AEfhorisHUD* HUD = CastChecked<AEfhorisHUD>(GetHUD()); IsValid(HUD))
		if (const UGameView* GameView = HUD->GetGameView(); IsValid(GameView))
			if (UMinimap* Minimap = GameView->GetMinimap(); IsValid(Minimap))
				Minimap->UpdatePlayers();
}

void AEfhorisPlayerController::BeginPlay()
{
	Super::BeginPlay();

	EnableCheats();
}

void AEfhorisPlayerController::OnPossess(APawn* InPawn) {
	Super::OnPossess(InPawn);

	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(InPawn))
	{
		Authority_UpdateHealthBars();
		Authority_UpdateMapNicknames();
	}

	Client_ResetHealthPotion();
}

void AEfhorisPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	using namespace EEndPlayReason;

	if (UEfhorisGameInstance* GameInstance = Cast<UEfhorisGameInstance>(GetGameInstance()); IsValid(GameInstance)) {
		if (GameInstance->IsDedicatedServerInstance()) {
			//UE_LOG(LogTemp, Error, TEXT("Serv EndPlayReason %d"), EndPlayReason)

			if ((EndPlayReason == Type::RemovedFromWorld) || (EndPlayReason == Type::Destroyed)) {
				Authority_UpdateHealthBars();
				Authority_UpdateMapNicknames();

				if (const UWorld* World = GetWorld(); IsValid(World))
					if (const AEfhorisGameState* GameState = World->GetGameState<AEfhorisGameState>(); IsValid(GameState))
						if (AAIManager* AIManager = GameState->GetAIManager(); IsValid(AIManager))
							AIManager->UpdateBossesScaling(GameState->GetNumberPlayer());
			}
		}
		else {
			//UE_LOG(LogTemp, Error, TEXT("Player EndPlayReason %d"), EndPlayReason)

			if((EndPlayReason == Type::Destroyed) || (EndPlayReason == Type::RemovedFromWorld))
				QuitGame(false);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AEfhorisPlayerController::AddOpenMenu() {
	NbOpenedMenus++;
	if (NbOpenedMenus ==1)
	{
		SetShowMouseCursor(true);
		SetInputMode(FInputModeGameAndUI{});
		Client_HideCrosshair();
	}
}

void AEfhorisPlayerController::RemoveOpenMenu() {
	NbOpenedMenus = FMath::Max(0,NbOpenedMenus-1);
	if (NbOpenedMenus == 0)
	{
		SetShowMouseCursor(false);
		SetInputMode(FInputModeGameOnly{});
		Client_ShowCrosshair();
	}
}

void AEfhorisPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

	if (ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(P))
	{
		BaseCharacter->InitializeAbilityActorInfo();
	}
}

void AEfhorisPlayerController::ClientRestart_Implementation(APawn* NewPawn)
{
	Super::ClientRestart_Implementation(NewPawn);
	if (ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(NewPawn); IsValid(BaseCharacter))
	{
		BaseCharacter->InitializeAbilityActorInfo();
	}
}

void AEfhorisPlayerController::Server_ResetMenuCooldown_Implementation() {
	MenuCooldown = 0;
}

void AEfhorisPlayerController::Server_IncrementMenuCooldown_Implementation(const bool bAddCooldown) {
	if (bAddCooldown) {
		MenuCooldown++;
	}
	else {
		MenuCooldown = FMath::Max(0,MenuCooldown -1);
	}
}

void AEfhorisPlayerController::Client_RefreshOpenedPanels_Implementation() {

	if (const AEfhorisHUD* HUD = CastChecked<AEfhorisHUD>(GetHUD()); IsValid(HUD))
		if (UGameView* GameView = HUD->GetGameView(); IsValid(GameView))
		{
			if (bChestOpened)
			{
				GameView->RefreshInventoryPanel();
				GameView->RefreshEquipmentPanel();
			}

			if (bIsShopPanelOpened)
				GameView->RefreshShopPanel();

			if (bIsContractPanelOpened)
				GameView->RefreshContractPanels();

			if(bIsMedicPanelOpened)
				GameView->RefreshMedicPanel();

			if (bIsSmithPanelOpened)
				GameView->RefreshSmithPanel();

			GameView->RefreshSideContractPanel();
		}
}

void AEfhorisPlayerController::Client_CloseInventoryPanel_Implementation()
{
	if (bChestOpened) {
		CloseInventoryPanel();
		bChestOpened = false;
	}
}

void AEfhorisPlayerController::Client_OpenInventoryPanel_Implementation()
{
	if(!bChestOpened) {
		OpenInventoryPanel();
		bChestOpened = true;
	}
}

void AEfhorisPlayerController::CloseInventoryPanel()
{
	if (const AEfhorisHUD* HUD = CastChecked<AEfhorisHUD>(GetHUD()); ensure(IsValid(HUD)))
	{
		if (const UGameView* GameView = HUD->GetGameView(); ensure(IsValid(GameView)))
		{
			if (UEfhorisPanel* InventoryPanel = GameView->GetInventoryPanel(); ensure(IsValid(InventoryPanel)))
				InventoryPanel->CloseMenu();

			if (UEfhorisPanel* EquipmentPanel = GameView->GetEquipmentPanel(); ensure(IsValid(EquipmentPanel)))
				EquipmentPanel->CloseMenu();
		}
	}
}

void AEfhorisPlayerController::Server_CallInventoryRequest_Implementation()
{
	if (HasAuthority())
	{
		if (const UWorld* World = CastChecked<UWorld>(GetWorld()); ensure(IsValid(World)))
		{
			if (const AEfhorisGameMode* EfhorisGameMode = CastChecked<AEfhorisGameMode>(World->GetAuthGameMode()); ensure(IsValid(EfhorisGameMode)))
			{
				if (APlayerCharacter* PlayerCharacter = CastChecked<APlayerCharacter>(GetCharacter()); ensure(IsValid(PlayerCharacter)))
				{
					if (PlayerCharacter->IsAliveTag())
					{
						EfhorisGameMode->CallInventory(PlayerCharacter);
					}
				}
			}
		}
	}
}

void AEfhorisPlayerController::Client_RefreshSpellsPanel_Implementation()
{
	if (const AEfhorisHUD* HUD = Cast<AEfhorisHUD>(GetHUD()); IsValid(HUD))
		if (UGameView* GameView = HUD->GetGameView(); IsValid(GameView))
			GameView->RefreshSpellsPanel();
}

void AEfhorisPlayerController::OpenInventoryPanel()
{
	if (const AEfhorisHUD* HUD = CastChecked<AEfhorisHUD>(GetHUD()); ensure(IsValid(HUD)))
	{
		if (const UGameView* GameView = HUD->GetGameView(); ensure(IsValid(GameView)))
		{
			if(UEfhorisPanel* InventoryPanel = GameView->GetInventoryPanel(); ensure(IsValid(InventoryPanel)))
			{
				InventoryPanel->RefreshPanel();
				InventoryPanel->OpenMenu();
			}
			if (UEfhorisPanel* EquipmentPanel = GameView->GetEquipmentPanel(); ensure(IsValid(EquipmentPanel)))
			{
				EquipmentPanel->RefreshPanel();
				EquipmentPanel->OpenMenu();
			}
		}
	}
}

void AEfhorisPlayerController::Client_OpenShopPanel_Implementation(AMerchant* Merchant)
{
	if (!bIsShopPanelOpened)
	{
		if (const AEfhorisHUD* EfhorisHUD = GetHUD<AEfhorisHUD>(); ensure(IsValid(EfhorisHUD)))
			if (const UGameView* GameView = EfhorisHUD->GetGameView(); ensure(IsValid(GameView)))
				if (UShopPanel* ShopPanel = Cast<UShopPanel>(GameView->GetShopPanel()); ensure(IsValid(ShopPanel)))
					ShopPanel->RefreshShopPanel(Merchant);

		OpenShopPanel();
		bIsShopPanelOpened = true;
	}
}

void AEfhorisPlayerController::Client_CloseShopPanel_Implementation() {
	if (bIsShopPanelOpened) {
		CloseInventoryPanel();
		bChestOpened = false;
		CloseShopPanel();
		bIsShopPanelOpened = false;
	}
}

void AEfhorisPlayerController::OpenShopPanel()
{
	if (const AEfhorisHUD* EfhorisHUD = GetHUD<AEfhorisHUD>(); ensure(IsValid(EfhorisHUD)))
	{
		if (const UGameView* GameView = EfhorisHUD->GetGameView(); ensure(IsValid(GameView)))
		{
			if (UEfhorisPanel* ShopPanel = GameView->GetShopPanel(); ensure(IsValid(ShopPanel)))
			{
				ShopPanel->OpenMenu();
				FAkAudioDevice::Get()->PostEvent(OpenShopPanelEvent, this);
			}
		}
	}
}

void AEfhorisPlayerController::CloseShopPanel()
{
	if (const AEfhorisHUD* EfhorisHUD = GetHUD<AEfhorisHUD>(); ensure(IsValid(EfhorisHUD)))
	{
		if (const UGameView* GameView = EfhorisHUD->GetGameView(); ensure(IsValid(GameView)))
		{
			if (UEfhorisPanel* ShopPanel = GameView->GetShopPanel(); ensure(IsValid(ShopPanel)))
			{
				ShopPanel->CloseMenu();
				FAkAudioDevice::Get()->PostEvent(ClosePanelEvent, this);
			}
		}
	}
}

void AEfhorisPlayerController::Client_OpenMedicPanel_Implementation()
{
	if (!bIsMedicPanelOpened)
	{
		if (const AEfhorisHUD* EfhorisHUD = GetHUD<AEfhorisHUD>(); ensure(IsValid(EfhorisHUD)))
			if (UGameView* GameView = EfhorisHUD->GetGameView(); ensure(IsValid(GameView)))
				GameView->RefreshMedicPanel();
		OpenMedicPanel();
		bIsMedicPanelOpened = true;
	}
}

void AEfhorisPlayerController::Client_CloseMedicPanel_Implementation()
{
	if (bIsMedicPanelOpened) {
		CloseMedicPanel();
		bIsMedicPanelOpened = false;
	}
}

void AEfhorisPlayerController::OpenMedicPanel()
{
	if (const AEfhorisHUD* EfhorisHUD = GetHUD<AEfhorisHUD>(); ensure(IsValid(EfhorisHUD)))
	{
		if (const UGameView* GameView = EfhorisHUD->GetGameView(); ensure(IsValid(GameView)))
		{
			if (UMedicPanel* MedicPanel = GameView->GetMedicPanel(); ensure(IsValid(MedicPanel)))
			{
				MedicPanel->OpenMenu();
				FAkAudioDevice::Get()->PostEvent(OpenMedicPanelEvent, this);
			}
		}
	}
}

void AEfhorisPlayerController::CloseMedicPanel()
{
	if (const AEfhorisHUD* EfhorisHUD = GetHUD<AEfhorisHUD>(); ensure(IsValid(EfhorisHUD)))
	{
		if (const UGameView* GameView = EfhorisHUD->GetGameView(); ensure(IsValid(GameView)))
		{
			if (UMedicPanel* MedicPanel = GameView->GetMedicPanel(); ensure(IsValid(MedicPanel)))
			{
				MedicPanel->CloseMenu();
				FAkAudioDevice::Get()->PostEvent(ClosePanelEvent, this);
			}
		}
	}
}

void AEfhorisPlayerController::OpenSmithPanel()
{
	if (const AEfhorisHUD* EfhorisHUD = GetHUD<AEfhorisHUD>(); ensure(IsValid(EfhorisHUD)))
	{
		if (const UGameView* GameView = EfhorisHUD->GetGameView(); ensure(IsValid(GameView)))
		{
			if (UEfhorisPanel* SmithPanel = GameView->GetSmithPanel(); ensure(IsValid(SmithPanel)))
			{
				SmithPanel->OpenMenu();
				FAkAudioDevice::Get()->PostEvent(OpenSmithPanelEvent, this);
			}
		}
	}
}

void AEfhorisPlayerController::CloseSmithPanel()
{
	if (const AEfhorisHUD* EfhorisHUD = GetHUD<AEfhorisHUD>(); ensure(IsValid(EfhorisHUD)))
	{
		if (const UGameView* GameView = EfhorisHUD->GetGameView(); ensure(IsValid(GameView)))
		{
			if (UEfhorisPanel* SmithPanel = GameView->GetSmithPanel(); ensure(IsValid(SmithPanel)))
			{
				SmithPanel->CloseMenu();
				FAkAudioDevice::Get()->PostEvent(ClosePanelEvent, this);
			}
		}
	}
}

void AEfhorisPlayerController::Client_CloseSmithPanel_Implementation()
{
	if (bIsSmithPanelOpened) {
		CloseSmithPanel();
		bIsSmithPanelOpened = false;
	}
}

void AEfhorisPlayerController::Client_OpenSmithPanel_Implementation()
{
	if(!bIsSmithPanelOpened)
	{
		if (const AEfhorisHUD* EfhorisHUD = GetHUD<AEfhorisHUD>(); ensure(IsValid(EfhorisHUD)))
			if (UGameView* GameView = EfhorisHUD->GetGameView(); ensure(IsValid(GameView)))
				GameView->RefreshSmithPanel();
		OpenSmithPanel();
		bIsSmithPanelOpened = true;
	}
}

void AEfhorisPlayerController::Client_OpenContractPanels_Implementation()
{
	if (!bIsContractPanelOpened)
	{
		OpenContractPanels();
		bIsContractPanelOpened = true;
	}
}

void AEfhorisPlayerController::Client_CloseContractPanels_Implementation()
{
	if (bIsContractPanelOpened)
	{
		CloseContractPanels();
		bIsContractPanelOpened = false;
	}
}

void AEfhorisPlayerController::OpenContractPanels()
{
	if (const AEfhorisHUD* HUD = CastChecked<AEfhorisHUD>(GetHUD()); ensure(IsValid(HUD)))
		if (const UGameView* GameView = HUD->GetGameView(); ensure(IsValid(GameView)))
			if (const AEfhorisGameState* GameState = GetWorld()->GetGameState<AEfhorisGameState>(); ensure(IsValid(GameState)))
			{
				if (UContractPanel* AccumulationContractPanel = Cast<UContractPanel>(GameView->GetAccumulationContractPanel()); ensure(IsValid(AccumulationContractPanel)))
				{
					AccumulationContractPanel->RefreshContractPanel(GameState->GetAccumulationContract());
					AccumulationContractPanel->OpenMenu();
				}

				if (UContractPanel* ExplorationContractPanel = Cast<UContractPanel>(GameView->GetExplorationContractPanel()); ensure(IsValid(ExplorationContractPanel)))			{
					ExplorationContractPanel->RefreshContractPanel(GameState->GetExplorationContract());
					ExplorationContractPanel->OpenMenu();
				}

				if (UContractPanel* FailableContractPanel = Cast<UContractPanel>(GameView->GetFailableContractPanel()); ensure(IsValid(FailableContractPanel)))
				{
					FailableContractPanel->RefreshContractPanel(GameState->GetFailableContract());
					FailableContractPanel->OpenMenu();
				}
				FAkAudioDevice::Get()->PostEvent(OpenContractPanelEvent, this);
			}
}

void AEfhorisPlayerController::CloseContractPanels()
{
	if (const AEfhorisHUD* HUD = CastChecked<AEfhorisHUD>(GetHUD()); ensure(IsValid(HUD)))
		if (const UGameView* GameView = HUD->GetGameView(); ensure(IsValid(GameView)))
		{
			if (UEfhorisPanel* AccumulationContractPanel = GameView->GetAccumulationContractPanel(); ensure(IsValid(AccumulationContractPanel)))
				AccumulationContractPanel->CloseMenu();

			if (UEfhorisPanel* ExplorationContractPanel = GameView->GetExplorationContractPanel(); ensure(IsValid(ExplorationContractPanel)))
				ExplorationContractPanel->CloseMenu();

			if (UEfhorisPanel* FailableContractPanel = GameView->GetFailableContractPanel(); ensure(IsValid(FailableContractPanel)))
				FailableContractPanel->CloseMenu();

			FAkAudioDevice::Get()->PostEvent(ClosePanelEvent, this);
		}
}

void AEfhorisPlayerController::Client_OnHitShake_Implementation(const float Scale) const {
	PlayerCameraManager->StartCameraShake(OnHitCameraShake, Scale);
}

void AEfhorisPlayerController::ShowInteractPrompt() const {
	if (const AEfhorisHUD* EfhorisHUD = GetHUD<AEfhorisHUD>(); IsValid(EfhorisHUD))
	{
		if (const UGameView* GameView = EfhorisHUD->GetGameView(); IsValid(GameView))
		{
			if (UInteractionPrompt* InteractPrompt = GameView->GetInteractPrompt(); IsValid(InteractPrompt))
			{
				InteractPrompt->SetVisibility(ESlateVisibility::Visible);
			}
		}
	}
}

void AEfhorisPlayerController::HideInteractPrompt() const {
	if (const AEfhorisHUD* EfhorisHUD = GetHUD<AEfhorisHUD>(); IsValid(EfhorisHUD))
	{
		if (const UGameView* GameView = EfhorisHUD->GetGameView(); IsValid(GameView))
		{
			if (UInteractionPrompt* InteractPrompt = GameView->GetInteractPrompt(); IsValid(InteractPrompt))
			{
				InteractPrompt->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
}

void AEfhorisPlayerController::Client_ShowDamageNumber_Implementation(ABaseCharacter* Target, const float DamageAmount) const {
	if (Target) {
		UDamageNumber* DamageText = NewObject<UDamageNumber>(Target, DamageNumberClass);
		DamageText->AddWorldTransform(FTransform(FVector(0, 0, Target->GetDamageNumberOffset())));
		DamageText->RegisterComponent();
		DamageText->AttachToComponent(Target->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		DamageText->SetDamageText(DamageAmount);
	}
}

void AEfhorisPlayerController::Authority_TeleportPlayer(const FVector& Vector) {
	GetPawn()->SetActorLocation(Vector);
}

void AEfhorisPlayerController::Client_ShowFloatingHealthBar_Implementation(const ABaseNPC* NPC, const bool bIsVisible) {
	if (IsValid(NPC)) {
		
 		NPC->SetHealthBarVisibility(bIsVisible);
	}
}

void AEfhorisPlayerController::Client_ResetHealthPotion_Implementation() const
{
	if (const AEfhorisHUD* HUD = Cast<AEfhorisHUD>(GetHUD()))
		if (UGameView* GameView = HUD->GetGameView())
			GameView->RefreshHealthPotion();
}

void AEfhorisPlayerController::Client_UpdatePlayer_Implementation() const
{
	if (const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn()))
		if (const AEfhorisHUD* HUD = Cast<AEfhorisHUD>(GetHUD()))
			if (const UGameView* GameView = HUD->GetGameView())
				GameView->GetHealthBar()->UpdatePlayer(PlayerCharacter);
}

void AEfhorisPlayerController::Authority_UpdateHealthBars() const {
	if (const UWorld* World = GetWorld(); IsValid(World))
		if (AEfhorisGameState* GameState = World->GetGameState<AEfhorisGameState>(); IsValid(GameState)) {
			TArray<FPlayerInfo> Descriptions;

			for (auto& Player_ : GameState->PlayerArray) {
				FPlayerInfo Description;

				if (const AEfhorisPlayerState* PlayerState_ = Cast<AEfhorisPlayerState>(Player_); IsValid(PlayerState_)) {
					if (APlayerCharacter* Character_ = PlayerState_->GetPawn<APlayerCharacter>(); IsValid(Character_)) {
						Description.Character = Character_;
						Description.NickName = PlayerState_->GetNickName();
					}
				}

				Descriptions.Add(Description);
			}

			if (AEfhorisGameMode* GameMode = World->GetAuthGameMode<AEfhorisGameMode>())
				GameMode->RefreshHealthPanel(Descriptions);
		}

	Client_UpdatePlayer();
}

void AEfhorisPlayerController::Authority_UpdateMapNicknames() const {
	if (const UWorld* World = GetWorld(); IsValid(World))
		if (AEfhorisGameState* GameState = World->GetGameState<AEfhorisGameState>(); IsValid(GameState)) {
			TArray<FPlayerInfo> Descriptions;

			for (auto& Player_ : GameState->PlayerArray) {
				FPlayerInfo Description;

				if (const AEfhorisPlayerState* PlayerState_ = Cast<AEfhorisPlayerState>(Player_); IsValid(PlayerState_)) {
					if (APlayerCharacter* Character_ = PlayerState_->GetPawn<APlayerCharacter>(); IsValid(Character_)) {
						Description.Character = Character_;
						Description.NickName = PlayerState_->GetNickName();
					}
				}

				Descriptions.Add(Description);
			}

			if (AEfhorisGameMode* GameMode = World->GetAuthGameMode<AEfhorisGameMode>())
				GameMode->RefreshMapNicknames(Descriptions);
		}
}

void AEfhorisPlayerController::Server_TrySmithItem_Implementation()
{
	if (AEfhorisGameMode* EfhorisGameMode = GetWorld()->GetAuthGameMode<AEfhorisGameMode>(); ensure(IsValid(EfhorisGameMode)))
		if(APlayerCharacter* PlayerCharacter = GetPawn<APlayerCharacter>(); ensure(IsValid(PlayerCharacter)))
				EfhorisGameMode->SmithPlayerItem(PlayerCharacter);
}

void AEfhorisPlayerController::Server_TryHealAllPlayers_Implementation()
{
	if (AEfhorisGameMode* EfhorisGameMode = GetWorld()->GetAuthGameMode<AEfhorisGameMode>(); ensure(IsValid(EfhorisGameMode)))
	{
		EfhorisGameMode->TryHealAllPlayers();
	}
}

void AEfhorisPlayerController::Client_RefreshSideContracts_Implementation()
{
	if (const AEfhorisHUD* HUD = Cast<AEfhorisHUD>(GetHUD()))
		if (UGameView* GameView = HUD->GetGameView())
			GameView->RefreshSideContractPanel();
}

void AEfhorisPlayerController::Server_AcceptContract_Implementation(AContract* Contract)
{
	if (!IsValid(Contract) || (Contract->GetStatus() != EContractStatus::NotStarted)) return;

	if (AEfhorisGameState* GameState = GetWorld()->GetGameState<AEfhorisGameState>(); IsValid(GameState))
	{
		const int NewMoney = GameState->GetMoney() - Contract->GetCost();

		if (NewMoney >= 0)
		{
			GameState->SetMoney(NewMoney);
			Contract->SetStatus(EContractStatus::OnGoing);
		}
	}
}

void AEfhorisPlayerController::Client_CooldownSpell_Implementation(float Cooldown, ESpellSlotType SpellType)
{
	if (const AEfhorisHUD* HUD = Cast<AEfhorisHUD>(GetHUD()); IsValid(HUD))
		if (UGameView* GameView = HUD->GetGameView(); IsValid(GameView))
			if (USpellSlot* Spell = GameView->GetSpell(SpellType); IsValid(Spell))
				Spell->CastSpell(Cooldown);
}

void AEfhorisPlayerController::Client_InitCooldownSpell_Implementation(ESpellSlotType SpellType)
{
	if (const AEfhorisHUD* HUD = Cast<AEfhorisHUD>(GetHUD()); IsValid(HUD))
		if (UGameView* GameView = HUD->GetGameView(); IsValid(GameView))
			if (USpellSlot* Spell = GameView->GetSpell(SpellType); IsValid(Spell))
				Spell->InitCooldown();
}

void AEfhorisPlayerController::Client_CancelCooldownSpell_Implementation(ESpellSlotType SpellType)
{
	if (const AEfhorisHUD* HUD = Cast<AEfhorisHUD>(GetHUD()); IsValid(HUD))
		if (UGameView* GameView = HUD->GetGameView(); IsValid(GameView))
			if (USpellSlot* Spell = GameView->GetSpell(SpellType); IsValid(Spell))
				Spell->CancelCooldown();
}

void AEfhorisPlayerController::ShowCreatorOptions() const
{
	if (const AEfhorisHUD* HUD = Cast<AEfhorisHUD>(GetHUD()); IsValid(HUD))
		if (const UGameView* GameView = HUD->GetGameView(); IsValid(GameView))
			if (UIngameMainMenu* IngameMenu = GameView->GetIngameMenu(); IsValid(IngameMenu))
				if (UToggleableMenu* GameOptions = IngameMenu->GetGameOptions(); IsValid(GameOptions))
					if (UOptions* Options = Cast<UOptions>(GameOptions); IsValid(Options))
						Options->OnCreatorSeted();
}

void AEfhorisPlayerController::Server_SetAutoSave_Implementation(bool bIsActivated) const
{
	USaveSubsystem* SaveSub = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	SaveSub->SetAutoSave(bIsActivated);
}

void AEfhorisPlayerController::SaveExit() const
{
	Server_SaveGame();
}

void AEfhorisPlayerController::Server_SaveGame_Implementation() const
{
	if (const AEfhorisPlayerState* PlayerState_ = GetPlayerState<AEfhorisPlayerState>(); IsValid(PlayerState_)) {
		USaveSubsystem* SaveSub = GetGameInstance()->GetSubsystem<USaveSubsystem>();
		SaveSub->SaveGame(PlayerState_->GetIsCreator());
	}
	Client_QuitGame();
}

void AEfhorisPlayerController::Client_QuitGame_Implementation() const
{
	QuitGame();
}

void AEfhorisPlayerController::QuitGame(const bool bShouldSave) const
{
	if (UEfhorisGameInstance* GameInstance = Cast<UEfhorisGameInstance>(GetGameInstance()); IsValid(GameInstance))
	{
		if (bShouldSave)
			if (const AEfhorisPlayerState* EfhorisPlayerState = GetPlayerState<AEfhorisPlayerState>(); IsValid(EfhorisPlayerState))
				EfhorisPlayerState->UploadStatistics();

		if (GameInstance->HasJoinedTheSession())
			GameInstance->DestroyServer();
		
		FAkAudioDevice::Get()->PostEventAtLocation(GameInstance->MenuMusic, FVector{ 0,0 ,0 }, FRotator{0,0,0}, GetWorld(), EAkAudioContext::GameplayAudio);
	}
}

void AEfhorisPlayerController::GenerateMinimap(const TArray<FLocatedTile>& Layout) {
	if (const AEfhorisHUD* EfhorisHUD = GetHUD<AEfhorisHUD>(); IsValid(EfhorisHUD)) {
		if (const UGameView* GameView = EfhorisHUD->GetGameView(); IsValid(GameView)) {
			if (UMinimap* Minimap = GameView->GetMinimap(); IsValid(Minimap)) {
				Minimap->GenerateMinimap(Layout);
			}
		}
	}
}

void AEfhorisPlayerController::PlaySafeZoneSequence()
{
	if (const auto& EfhorisGameInstance = GetGameInstance<UEfhorisGameInstance>(); IsValid(EfhorisGameInstance))
	{
		Client_StopAllMusic();
		Client_PlayEvent(EfhorisGameInstance->SafeZoneMusic);
		Client_PlayEvent(EfhorisGameInstance->AmbianceMusic);
		bHasLeftSafeZone = false;
		Client_PlaySequence(EfhorisGameInstance->GetStartTileSequence());
	}
}

void AEfhorisPlayerController::PlayEndSequence()
{
	ALevelSequenceActor* LevelSequenceActor;


	FMovieSceneSequencePlaybackSettings Settings;
	Settings.bHideHud = true;
	Settings.bHidePlayer = true;

	LevelSequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
		GetWorld(),
		GetGameInstance<UEfhorisGameInstance>()->GetEndGameSequence(), Settings,
		LevelSequenceActor);
	if (LevelSequencePlayer)
	{
		//Hide hud
		AEfhorisHUD* EfhorisHUD = GetHUD<AEfhorisHUD>();
		EfhorisHUD->HideGameView();


		//ICI METTRE UN POST EVENT AVEC VICTOIRE DE OUF 

		
		LevelSequencePlayer->Play();

		//Show Gameview after start sequence
		LevelSequencePlayer->OnFinished.AddDynamic(EfhorisHUD, &AEfhorisHUD::Win);
	}
}

void AEfhorisPlayerController::SkipSequence()
{
	if(IsValid(LevelSequencePlayer))
		if(LevelSequencePlayer->IsPlaying())
		{
			LevelSequencePlayer->GoToEndAndStop();
			GetHUD<AEfhorisHUD>()->ShowGameView();
		}
}

void AEfhorisPlayerController::PlayCallInventorySound()
{
	FAkAudioDevice::Get()->PostEvent(CallInventorySound, this);
}

void AEfhorisPlayerController::Client_EnterSafeZone_Implementation()
{
	auto PlayMusic = [&]() {
		if (UEfhorisGameInstance* GameInstance = GetWorld()->GetGameInstance<UEfhorisGameInstance>(); IsValid(GameInstance))
			Client_PlayEvent(GameInstance->SafeZoneMusic);
		bHasLeftSafeZone = false;
	};

	if (bHasLeftSafeZone)  PlayMusic();
	else if(!bHasPlaySafeZoneSequence) {
		PlayMusic();
		PlaySafeZoneSequence();
		bHasPlaySafeZoneSequence = true;
	}
}

void AEfhorisPlayerController::Client_LeftSafeZone_Implementation()
{
	if (!bHasLeftSafeZone) {
		bHasLeftSafeZone = bHasPlaySafeZoneSequence = true;

		if (UEfhorisGameInstance* GameInstance = GetWorld()->GetGameInstance<UEfhorisGameInstance>(); IsValid(GameInstance))
		{
			Client_PlayEvent(GameInstance->StartLevelMusic);
		}
		Client_SetMusicState(FName("PalierX"), FName("Hors_Combat"));
	}
}

void AEfhorisPlayerController::Client_StopAllMusic_Implementation()
{
	FAkAudioDevice::Get()->StopAllSounds();
}

void AEfhorisPlayerController::Client_PlayEvent_Implementation(UAkAudioEvent* MusicToPlay)
{
	FAkAudioDevice::Get()->PostEvent(MusicToPlay, this);
}

void AEfhorisPlayerController::Client_SetMusicState_Implementation(FName StateGroup, FName NewState)
{
	FAkAudioDevice::Get()->SetState(*StateGroup.ToString(), *NewState.ToString());
}

void AEfhorisPlayerController::Client_PlaySequence_Implementation(ULevelSequence* SequenceToPlay, bool bShouldPlayStartSequenceAfter)
{
	ALevelSequenceActor* LevelSequenceActor;

	FMovieSceneSequencePlaybackSettings Settings;
	Settings.bHideHud = true;
	Settings.bHidePlayer = true;

	LevelSequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
		GetWorld(),
		SequenceToPlay, Settings,
		LevelSequenceActor);
	if(LevelSequencePlayer)
	{
		//Hide hud
		AEfhorisHUD* EfhorisHUD = GetHUD<AEfhorisHUD>();
		EfhorisHUD->HideGameView();

		LevelSequencePlayer->Play();
		if(bShouldPlayStartSequenceAfter)
		{
			//Show Gameview after start sequence
			LevelSequencePlayer->OnFinished.AddDynamic(this, &AEfhorisPlayerController::PlaySafeZoneSequence);
		}
		else
		{
			//Show Gameview after this sequence
			LevelSequencePlayer->OnFinished.AddDynamic(EfhorisHUD, &AEfhorisHUD::ShowGameView);
		}
	}
}

void AEfhorisPlayerController::Client_HideChestMoving_Implementation() const {
	if (const AEfhorisHUD* HUD = CastChecked<AEfhorisHUD>(GetHUD()); IsValid(HUD))
		if (const UGameView* GameView = HUD->GetGameView(); IsValid(GameView))
			if (UUserWidget* Chest = GameView->GetChestMoving(); IsValid(Chest))
				Chest->SetVisibility(ESlateVisibility::Hidden);
}

void AEfhorisPlayerController::Client_ShowChestMoving_Implementation() const {
	if (const AEfhorisHUD* HUD = CastChecked<AEfhorisHUD>(GetHUD()); IsValid(HUD))
		if (const UGameView* GameView = HUD->GetGameView(); IsValid(GameView))
			if (UUserWidget* Chest = GameView->GetChestMoving(); IsValid(Chest))
				Chest->SetVisibility(ESlateVisibility::Visible);
}

void AEfhorisPlayerController::Client_HideCrosshair_Implementation() const {
	if (const AEfhorisHUD* HUD = CastChecked<AEfhorisHUD>(GetHUD()); IsValid(HUD))
		if (const UGameView* GameView = HUD->GetGameView(); IsValid(GameView))
			if (UUserWidget* Crosshair = GameView->GetCrosshair(); IsValid(Crosshair))
				Crosshair->SetVisibility(ESlateVisibility::Hidden);
}

void AEfhorisPlayerController::Client_ShowCrosshair_Implementation() const {
	if (bIsMinimapOpened || NbOpenedMenus > 0) return;
	if (const AEfhorisHUD* HUD = CastChecked<AEfhorisHUD>(GetHUD()); IsValid(HUD))
		if (const UGameView* GameView = HUD->GetGameView(); IsValid(GameView))
			if (UUserWidget* Crosshair = GameView->GetCrosshair(); IsValid(Crosshair))
				Crosshair->SetVisibility(ESlateVisibility::Visible);
}

void AEfhorisPlayerController::Client_ForceCloseAllMenus_Implementation(const bool bPlayAnimation) {
	if (const AEfhorisHUD* EfhorisHUD = GetHUD<AEfhorisHUD>(); IsValid(EfhorisHUD)) {
		if (const UGameView* GameView = EfhorisHUD->GetGameView(); IsValid(GameView)) {

			if (UToggleableMenu* Menu = GameView->GetEquipmentPanel(); IsValid(Menu))
				Menu->ForceCloseMenu(bPlayAnimation);
			if (UToggleableMenu* Menu = GameView->GetInventoryPanel(); IsValid(Menu))
				Menu->ForceCloseMenu(bPlayAnimation);
			if (UToggleableMenu* Menu = GameView->GetMedicPanel(); IsValid(Menu))
				Menu->ForceCloseMenu(bPlayAnimation);
			if (UToggleableMenu* Menu = GameView->GetShopPanel(); IsValid(Menu))
				Menu->ForceCloseMenu(bPlayAnimation);
			if (UToggleableMenu* Menu = GameView->GetSmithPanel(); IsValid(Menu))
				Menu->ForceCloseMenu(bPlayAnimation);
			if (UToggleableMenu* Menu = GameView->GetAccumulationContractPanel(); IsValid(Menu))
				Menu->ForceCloseMenu(bPlayAnimation);
			if (UToggleableMenu* Menu = GameView->GetExplorationContractPanel(); IsValid(Menu))
				Menu->ForceCloseMenu(bPlayAnimation);
			if (UToggleableMenu* Menu = GameView->GetFailableContractPanel(); IsValid(Menu))
				Menu->ForceCloseMenu(bPlayAnimation);
		}
	}
	if (const APlayerCharacter* ControlledPlayer = Cast<APlayerCharacter>(GetPawn()); IsValid(ControlledPlayer))
		ControlledPlayer->GetInteractionComponent()->Server_ForceStopInteract();
	Server_ResetMenuCooldown();
}

void AEfhorisPlayerController::Client_Win_Implementation() {
	PlayEndSequence();
}

void AEfhorisPlayerController::Client_Lose_Implementation() {
	Client_StopAllMusic();
	Client_PlayEvent(PlayerLoseEvent);
	Cast<AEfhorisHUD>(GetHUD())->ShowGameView();
	Cast<AEfhorisHUD>(GetHUD())->GetGameView()->Lose();
}