#include "EfhorisGameState.h"

#include "GameFramework/HUD.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "ProjetHiver/BDD/DataBaseSubsystem.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/Characters/AttributeSets/HealthAttributeSet.h"
#include "ProjetHiver/Contracts/ContractStatus.h"
#include "ProjetHiver/Contracts/FailableContractType.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerState.h"
#include "ProjetHiver/GAS/EfhorisAbilitySystemComponent.h"
#include "ProjetHiver/Inventory/InventoryCharacter.h"
#include "ProjetHiver/Items/ItemInfo.h"
#include "ProjetHiver/Items/PickableItem.h"
#include "ProjetHiver/MapGeneration/LevelGenerator.h"
#include "ProjetHiver/Save/GameSave.h"
#include "ProjetHiver/Widgets/GameView.h"

FPlayersAverageEquipment::FPlayersAverageEquipment(const float _AverageArmorLevel, const float _AverageWeaponLevel,
                                                   const FItemStats _AverageArmorStats, const FItemStats _AverageWeaponStats)
	: AverageArmorLevel{ _AverageArmorLevel }, AverageWeaponLevel{ _AverageWeaponLevel }, AverageArmorStats{ _AverageArmorStats }, AverageWeaponStats{ _AverageWeaponStats } {}


void AEfhorisGameState::GenerateLayoutMinimap()
{
	LayoutMinimap = LevelGenerator->GenerateArrayFromLayoutMap();
}

void AEfhorisGameState::OnRep_LayoutMinimapRep() const
{
	if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
		if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(World->GetFirstPlayerController()); IsValid(PlayerController))
			if(IsValid(PlayerController->Player))
				PlayerController->GenerateMinimap(LayoutMinimap);
}

AEfhorisGameState::AEfhorisGameState() : AGameStateBase() {
	GameId = FGuid::NewGuid();

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;
}

void AEfhorisGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsValid(FailableContract))
		if (FailableContract->GetStatus() == EContractStatus::OnGoing && FailableContract->GetType() == EFailableContractType::Time)
		{
			if (!bTimeContractStarted)
			{
				bTimeContractStarted = true;
				TimeContractStartTime = GetLevelTimer();
			}

			if (bTimeContractStarted)
				FailableContract->SetCurrentValue(GetLevelTimer() - TimeContractStartTime);
		}
}

void AEfhorisGameState::StoreItem(APickableItem* Item)
{
	const FItemInfo NewItemInfo = Item->GetItemInfo();

	bool bHadToBeAdded = true;

	if (NewItemInfo.ItemType == EItemType::Trash)
		for (FItemInfo& Info : SharedInventory)
			if (Info.RowName == NewItemInfo.RowName)
			{
				bHadToBeAdded = false;
				++Info.Stack;
				break;
			}

	if (bHadToBeAdded)
		SharedInventory.Add(NewItemInfo);

	Item->Destroy();
}

void AEfhorisGameState::StoreItemInfo(FItemInfo ItemInfo)
{
	bool bHadToBeAdded = true;

	if (ItemInfo.ItemType == EItemType::Trash)
		for (FItemInfo& Info : SharedInventory)
			if (Info.RowName == ItemInfo.RowName)
			{
				bHadToBeAdded = false;
				++Info.Stack;
				break;
			}

	if (bHadToBeAdded)
		SharedInventory.Add(ItemInfo);
}

void AEfhorisGameState::RemoveItemInfo(FItemInfo Item)
{
	SharedInventory.Remove(Item);
}

bool AEfhorisGameState::ContainsItem(const FItemInfo& Item) const
{
	return SharedInventory.Contains(Item);
}

void AEfhorisGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AEfhorisGameState, SharedInventory);
	DOREPLIFETIME(AEfhorisGameState, Inventory);
	DOREPLIFETIME(AEfhorisGameState, Money);
	DOREPLIFETIME(AEfhorisGameState, LevelStartTime);
	DOREPLIFETIME(AEfhorisGameState, bGameEnded);
	DOREPLIFETIME(AEfhorisGameState, AccumulationContract);
	DOREPLIFETIME(AEfhorisGameState, ExplorationContract);
	DOREPLIFETIME(AEfhorisGameState, FailableContract);
	DOREPLIFETIME(AEfhorisGameState, HealAllPlayersCost);
	DOREPLIFETIME(AEfhorisGameState, LayoutMinimap);
}

void AEfhorisGameState::RefreshUI() const
{
	if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
		if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(World->GetFirstPlayerController()); IsValid(PlayerController))
			PlayerController->Client_RefreshOpenedPanels();
}

void AEfhorisGameState::OnRep_SharedInventoryRep() const
{
	RefreshUI();
}

void AEfhorisGameState::OnRep_MoneyRep() const
{
	RefreshUI();
}

void AEfhorisGameState::OnRep_ContractRep() const
{
	if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
		if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(World->GetFirstPlayerController()); IsValid(PlayerController))
			PlayerController->Client_RefreshSideContracts();
}

void AEfhorisGameState::OnRep_MedicRep() const
{
	RefreshUI();
}

int32 AEfhorisGameState::GetCurrentLevel() const noexcept
{
	return LevelGenerator->CurrentLevel;
}

bool AEfhorisGameState::IsLevelGeneratorValid() const
{
	return IsValid(LevelGenerator); 
}

AInventoryCharacter* AEfhorisGameState::GetInventory() const noexcept
{
	return Inventory;
}

void AEfhorisGameState::SetInventory(AInventoryCharacter* Inventory_) noexcept
{
	Inventory = Inventory_;
}

float AEfhorisGameState::GetPlayersMaxHealth() const noexcept
{
	float Sum = 0.f;
	
	for (const auto Player : PlayerArray) {
		if(const APlayerCharacter* Char = Cast<APlayerCharacter>(Player->GetPawn()); IsValid(Char))
			Sum += Char->GetAbilitySystemComponent()->GetNumericAttribute(UHealthAttributeSet::GetMaxHealthAttribute());
	}
	return Sum;
}

float AEfhorisGameState::GetPlayersCurrentHealth() const noexcept
{
	float Sum = 0.f;

	for (const auto Player : PlayerArray) {
		if (const APlayerCharacter* Char = Cast<APlayerCharacter>(Player->GetPawn()); IsValid(Char))
			Sum += Char->GetAbilitySystemComponent()->GetNumericAttribute(UHealthAttributeSet::GetHealthAttribute());
	}
	return Sum;
}

FString AEfhorisGameState::GetGameName() const noexcept {
	if(IsValid(Creator))
		return GameName.IsEmpty() ? "Partie de " + Creator->GetNickName() : GameName;

	return {};
}

FString AEfhorisGameState::GetGameCreatorId() const noexcept
{
	if (IsValid(Creator)) {
		return Creator->GetSteamUserId();
	}

	return {};
}

void AEfhorisGameState::SetGameCreatorId(const FString& NewGameCreatorId) noexcept {
	for (auto& Player : PlayerArray) {
		if (AEfhorisPlayerState* PlayerState = Cast<AEfhorisPlayerState>(Player); IsValid(PlayerState)) {
			if (PlayerState->GetIsCreator() && PlayerState->GetSteamUserId() == NewGameCreatorId) {
				Creator = PlayerState;
				break;
			}
		}
	}
}

int AEfhorisGameState::GetNumberPlayerAlive() const
{
	int NbPlayerAlive = 0;
	for(const auto PlayerState : PlayerArray)
	{
		if (const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(PlayerState->GetPawn()); IsValid(PlayerCharacter))
		{
			if(PlayerCharacter->IsAliveHealth())
			{
				++NbPlayerAlive;
			}
		}
	}
	return NbPlayerAlive;
}

FPlayersAverageEquipment AEfhorisGameState::GetAveragePlayersEquipment()
{
	FPlayersAverageEquipment PlayersAverageEquipment{};

	for (const auto PlayerState : PlayerArray)
	{
		if (const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(PlayerState->GetPawn()); IsValid(PlayerCharacter))
		{
			if (PlayerCharacter->HasWeaponKit())
			{
				PlayersAverageEquipment.AverageWeaponLevel += static_cast<float>(PlayerCharacter->GetWeaponKitInfo().Level);
				PlayersAverageEquipment.AverageWeaponStats = PlayersAverageEquipment.AverageWeaponStats + PlayerCharacter->GetWeaponKitInfo().ItemStats;
			}
			if (PlayerCharacter->HasArmorKit())
			{
				PlayersAverageEquipment.AverageArmorLevel += static_cast<float>(PlayerCharacter->GetArmorKitInfo().Level);
				PlayersAverageEquipment.AverageArmorStats = PlayersAverageEquipment.AverageArmorStats + PlayerCharacter->GetArmorKitInfo().ItemStats;
			}
		}
	}

	if(const int NbPlayers = GetNumberPlayer(); NbPlayers > 0)
	{
		PlayersAverageEquipment.AverageWeaponLevel /= NbPlayers;
		PlayersAverageEquipment.AverageArmorLevel /= NbPlayers;

		PlayersAverageEquipment.AverageWeaponStats = PlayersAverageEquipment.AverageWeaponStats / NbPlayers;
		PlayersAverageEquipment.AverageArmorStats = PlayersAverageEquipment.AverageArmorStats / NbPlayers;
	}

	return PlayersAverageEquipment;
}

float AEfhorisGameState::GetLevelTimer() const
{
	const float TimeNow = UGameplayStatics::GetRealTimeSeconds(GetWorld());
	return TimeNow - LevelStartTime;
}

void AEfhorisGameState::Authority_ResetLevelTimer()
{
	LevelStartTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
}

void AEfhorisGameState::SetMoney(const int _Money) noexcept
{
	if (HasAuthority()) Money = _Money;
}

FGameSave AEfhorisGameState::GetGameStateSave() const noexcept
{
	FGameSave GameSave;
	GameSave.GameId = GetGameId();
	GameSave.GameCreatorId = GetGameCreatorId();
	GameSave.GameName = GetGameName();
	GameSave.Date = FDateTime::Now();

	GameSave.SharedInventory = GetSharedInventory();
	GameSave.Money = GetMoney();
	if (IsValid(Inventory)) {
		GameSave.InventoryTransform = Inventory->GetActorTransform();
		GameSave.TargetId = Inventory->GetTargetId();
	}
	
	if (IsLevelGeneratorValid()) {
		GameSave.Seed = GetLevelGenerator()->Seed;
		GameSave.CurrentLevel = GetLevelGenerator()->CurrentLevel;
	}

	GameSave.DamageTaken = GetDamageTaken();
	GameSave.NbVisitedTiles = GetNbVisitedTiles();

	GameSave.ContractClassPaths.Add(AccumulationContract->GetClass()->GetClassPathName());
	GameSave.ContractClassPaths.Add(ExplorationContract->GetClass()->GetClassPathName());
	GameSave.ContractClassPaths.Add(FailableContract->GetClass()->GetClassPathName());

	GameSave.ContractStatuses.Add(AccumulationContract->GetStatus());
	GameSave.ContractStatuses.Add(ExplorationContract->GetStatus());
	GameSave.ContractStatuses.Add(FailableContract->GetStatus());

	GameSave.ContractCurrentValues.Add(AccumulationContract->GetCount());
	// Exploration contracts don't have a current value or a count
	GameSave.ContractCurrentValues.Add(FailableContract->GetCurrentValue());

	GameSave.HealAllPlayersCost = HealAllPlayersCost;

	return GameSave;
}

void AEfhorisGameState::SetGameStateSave(FGameSave& GameSave) noexcept
{
	SetGameId(GameSave.GameId);
	SetGameName(GameSave.GameName);
	SetGameCreatorId(GameSave.GameCreatorId);

	SharedInventory.Reserve(GameSave.SharedInventory.Num());
	for (auto& ItemInfo : GameSave.SharedInventory) {
		ItemInfo.Ability1 = FindObject<UClass>(ItemInfo.Ability1ClassPath);
		ItemInfo.Ability2 = FindObject<UClass>(ItemInfo.Ability2ClassPath);
		SharedInventory.Add(ItemInfo);
	}
	SetMoney(GameSave.Money);
	if (IsValid(Inventory)) {
		Inventory->SetActorTransform(GameSave.InventoryTransform);
		Inventory->SetTargetId(GameSave.TargetId);
	}

	GetLevelGenerator()->Seed = GameSave.Seed;
	GetLevelGenerator()->CurrentLevel = GameSave.CurrentLevel;

	DamageTaken = GameSave.DamageTaken;
	NbVisitedTiles = GameSave.NbVisitedTiles;

	if (AEfhorisGameMode* GameMode = GetWorld()->GetAuthGameMode<AEfhorisGameMode>(); IsValid(GameMode))
	{
		AccumulationContract = GameMode->SpawnActor<AAccumulationContract>(FindObject<UClass>(GameSave.ContractClassPaths[0]));
		ExplorationContract = GameMode->SpawnActor<AExplorationContract>(FindObject<UClass>(GameSave.ContractClassPaths[1]));
		FailableContract = GameMode->SpawnActor<AFailableContract>(FindObject<UClass>(GameSave.ContractClassPaths[2]));

		AccumulationContract->SetStatus(GameSave.ContractStatuses[0]);
		ExplorationContract->SetStatus(GameSave.ContractStatuses[1]);
		FailableContract->SetStatus(GameSave.ContractStatuses[2]);

		AccumulationContract->SetCount(GameSave.ContractCurrentValues[0]);
		// Exploration contracts don't have a current value or a count
		FailableContract->SetCurrentValue(GameSave.ContractCurrentValues[1]);
	}

	HealAllPlayersCost = GameSave.HealAllPlayersCost;
}

AAccumulationContract* AEfhorisGameState::GetAccumulationContract() const noexcept
{
	return AccumulationContract;
}

void AEfhorisGameState::SetAccumulationContract(AAccumulationContract* Contract) noexcept
{
	if (IsValid(AccumulationContract)) AccumulationContract->Destroy();

	AccumulationContract = Contract;
}

AExplorationContract* AEfhorisGameState::GetExplorationContract() const noexcept
{
	return ExplorationContract;
}

void AEfhorisGameState::SetExplorationContract(AExplorationContract* Contract) noexcept
{
	if (IsValid(ExplorationContract)) ExplorationContract->Destroy();

	ExplorationContract = Contract;
}

AFailableContract* AEfhorisGameState::GetFailableContract() const noexcept
{
	return FailableContract;
}

void AEfhorisGameState::SetFailableContract(AFailableContract* Contract) noexcept
{
	if (IsValid(FailableContract)) FailableContract->Destroy();

	FailableContract = Contract;
	bTimeContractStarted = false;
}

void AEfhorisGameState::IsTargetedByInventory(const AEfhorisPlayerState* Player)
{
	if (IsValid(Inventory))
		if (Inventory->HasTargetId() && !Inventory->HasTarget() && (Inventory->GetTargetId() == Player->GetSteamUserId()))
			if (APlayerCharacter* Player_ = Cast<APlayerCharacter>(Player->GetPawn()); IsValid(Player_))
				Inventory->SetTarget(Player_);
}
