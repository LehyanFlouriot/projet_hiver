#include "EfhorisGameMode.h"

#include "AbilitySystemComponent.h"
#include "EfhorisPlayerState.h"
#include "SocketSubsystem.h"
#include "Engine/CompositeDataTable.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "ProjetHiver/AI/AIDirector.h"
#include "ProjetHiver/AI/AIManager.h"
#include "ProjetHiver/BDD/DataBaseSubsystem.h"
#include "ProjetHiver/BDD/UserScoreboardStatistic.h"
#include "ProjetHiver/Characters/DragonBoss.h"
#include "ProjetHiver/Characters/Mercenary.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/Contracts/AccumulationContract.h"
#include "ProjetHiver/Contracts/ContractStatus.h"
#include "ProjetHiver/Contracts/ExplorationContract.h"
#include "ProjetHiver/Contracts/FailableContract.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameInstance.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"
#include "ProjetHiver/Inventory/InventoryCharacter.h"
#include "ProjetHiver/Inventory/Merchant.h"
#include "ProjetHiver/Items/HealSeed.h"
#include "ProjetHiver/Items/ItemInfo.h"
#include "ProjetHiver/Items/ItemType.h"
#include "ProjetHiver/Items/PickableItem.h"
#include "ProjetHiver/Items/ItemRows/ArmorKitRow.h"
#include "ProjetHiver/Items/ItemRows/BeltRow.h"
#include "ProjetHiver/Items/ItemRows/EarringsRow.h"
#include "ProjetHiver/Items/ItemRows/GlovesRow.h"
#include "ProjetHiver/Items/ItemRows/NecklaceRow.h"
#include "ProjetHiver/Items/ItemRows/RingRow.h"
#include "ProjetHiver/Items/ItemRows/TrashItemRow.h"
#include "ProjetHiver/Items/ItemRows/WeaponKitRow.h"
#include "ProjetHiver/MapGeneration/LevelGenerator.h"
#include "ProjetHiver/Save/GameSave.h"
#include "ProjetHiver/Save/SaveSubsystem.h"
#include "ProjetHiver/Widgets/GameView.h"
#include "SocketSubsystem.h"
#include "ProjetHiver/Characters/Enemy.h"
#include "ProjetHiver/Items/ItemRows/BeltRow.h"
#include "ProjetHiver/Items/ItemRows/EarringsRow.h"
#include "ProjetHiver/Items/ItemRows/GlovesRow.h"
#include "ProjetHiver/Items/ItemRows/NecklaceRow.h"
#include "ProjetHiver/Items/ItemRows/RingRow.h"

void AEfhorisGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (const UWorld* World = GetWorld(); IsValid(World))
		if (UEfhorisGameInstance* GameInstance = Cast<UEfhorisGameInstance>(World->GetGameInstance()); IsValid(GameInstance)) {
			if (AEfhorisGameState* GameState_ = GetGameState<AEfhorisGameState>(); ensure(IsValid(GameState_))) {
				// Renommer la partie
				GameState_->SetGameName(GameInstance->GetGameSave().GameName);

				// Get LevelGenerator
				ALevelGenerator* LevelGenerator = Cast<ALevelGenerator>(UGameplayStatics::GetActorOfClass(GetWorld(), ALevelGenerator::StaticClass()));
				GameState_->SetLevelGenerator(LevelGenerator);

				// Create Chest
				GameState_->SetInventory(SpawnActor<AInventoryCharacter>(InventoryClass, GameInstance->GetGameSave().InventoryTransform));

				// Load the save
				if (GameInstance->ShouldLoadGame()) {
					GameState_->SetGameStateSave(GameInstance->GetGameSave());
				}

				// Create AI Director & AI Manager
				AAIDirector* Director = SpawnActor<AAIDirector>(AIDirectorClass);
				GameState_->SetAIDirector(Director);
				AAIManager* Manager = SpawnActor<AAIManager>(AIManagerClass);
				GameState_->SetAIManager(Manager);
				Director->SetAIManager(Manager);
				Director->SetLvlGenerator(LevelGenerator);

				if (GameState_->IsLevelGeneratorValid()) {
					if (!GameInstance->ShouldLoadGame())
						GenerateContracts();
					GameState_->GetLevelGenerator()->ResetLevel(false);
					GameState_->GenerateLayoutMinimap();
					if (GameInstance->ShouldLoadGame())
						Director->SetAIDirectorSave(GameInstance->GetDirectorSave());
					else {
						GameState_->ResetHealCost();
						Director->Update(true);
					}
				}
			}
		}
}

void AEfhorisGameMode::Logout(AController* Exiting)
{
	if (const AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(Exiting); IsValid(PlayerController)) {
		if (const AEfhorisGameState* GameState_ = GetGameState<AEfhorisGameState>(); IsValid(GameState_)) {
			bool bIsLast = GameState_->PlayerArray.Num() == 1;
			if (const AEfhorisPlayerState* PlayerState = PlayerController->GetPlayerState<AEfhorisPlayerState>(); IsValid(PlayerState)) {
				if (PlayerState->GetIsCreator() || bIsLast) {
					// Le createur est en train de quitter le lobby
					// On reset donc le dedicated Serv
					ResetDedicatedServ();
				}
			}
		}
	}

	Super::Logout(Exiting);
}

void AEfhorisGameMode::ResetDedicatedServ()
{
	if (UWorld* World = GetWorld(); IsValid(World)) {
		if (UEfhorisGameInstance* GameInstance = Cast<UEfhorisGameInstance>(GetGameInstance()); IsValid(GameInstance)) {
			if (GameInstance->IsDedicatedServerInstance()) {
				USaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<USaveSubsystem>();

				SaveSubsystem->ResetSaves();
				GameInstance->ResetShouldLoadGame();

				// Deconnecter tous les joueurs
				if (const AEfhorisGameState* GameState_ = GetGameState<AEfhorisGameState>(); IsValid(GameState_)) {
					for (auto& PlayerState : GameState_->PlayerArray) {
						if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(PlayerState->GetPlayerController()); IsValid(PlayerController)) {
							PlayerController->Client_QuitGame();
						}
					}
				}

				World->ServerTravel("/Game/Efhoris/Multiplayer/CreateMainMenu?listen");
			}
		}
	}
}

int AEfhorisGameMode::ScaleValueWithLevel(const int BaseValue, const int Level, const float Coefficient)
{
	return BaseValue + static_cast<int>(static_cast<float>(BaseValue) * Coefficient * static_cast<float>(Level - 1));
}

float AEfhorisGameMode::ScaleFValueWithLevel(const float BaseValue, const int Level, const float Coefficient)
{
	return BaseValue + BaseValue * Coefficient * static_cast<float>(Level - 1);
}

int AEfhorisGameMode::RandomizeValueWithRange(const int BaseValue, const float Range)
{
	return BaseValue * FMath::RandRange(1.f - Range, 1.f + Range);
}

float AEfhorisGameMode::RandomizeFValueWithRange(const float BaseValue, const float Range)
{
	return BaseValue * FMath::RandRange(1.f - Range, 1.f + Range);
}

void AEfhorisGameMode::ScaleItemInfo(FItemInfo& ItemInfo) const
{
	ItemInfo.BuyValue = ScaleValueWithLevel(ItemInfo.BuyValue, ItemInfo.Level, BuyValueScaling);
	ItemInfo.SellValue = ScaleValueWithLevel(ItemInfo.SellValue, ItemInfo.Level, SellValueScaling);
	ItemInfo.ItemStats = FItemStats{
		ScaleValueWithLevel(ItemInfo.ItemStats.PhysicalPower, ItemInfo.Level, PhysicalPowerScaling),
		ScaleValueWithLevel(ItemInfo.ItemStats.PhysicalDefense, ItemInfo.Level, PhysicalDefenseScaling),
		ScaleValueWithLevel(ItemInfo.ItemStats.BlockingPhysicalDefense, ItemInfo.Level, BlockingPhysicalDefenseScaling),
		ScaleValueWithLevel(ItemInfo.ItemStats.MagicalPower, ItemInfo.Level, MagicalPowerScaling),
		ScaleValueWithLevel(ItemInfo.ItemStats.MagicalDefense, ItemInfo.Level, MagicalDefenseScaling),
		ScaleValueWithLevel(ItemInfo.ItemStats.BlockingMagicalDefense, ItemInfo.Level, BlockingMagicalDefenseScaling),
		ScaleValueWithLevel(ItemInfo.ItemStats.MaxHealth, ItemInfo.Level, MaxHealthScaling),
		ItemInfo.ItemStats.MaxPotionCharge,
		ItemInfo.ItemStats.MovementSpeed == 1 ?
			ItemInfo.ItemStats.MovementSpeed :
			static_cast<int>(ScaleFValueWithLevel(ItemInfo.ItemStats.MovementSpeed, ItemInfo.Level, MovementSpeedScaling))
	};
}

void AEfhorisGameMode::RandomizeItemInfo(FItemInfo& ItemInfo) const
{
	ItemInfo.ItemStats = FItemStats{
		RandomizeValueWithRange(ItemInfo.ItemStats.PhysicalPower, PhysicalPowerRandom),
		RandomizeValueWithRange(ItemInfo.ItemStats.PhysicalDefense, PhysicalDefenseRandom),
		RandomizeValueWithRange(ItemInfo.ItemStats.BlockingPhysicalDefense, BlockingPhysicalDefenseRandom),
		RandomizeValueWithRange(ItemInfo.ItemStats.MagicalPower, MagicalPowerRandom),
		RandomizeValueWithRange(ItemInfo.ItemStats.MagicalDefense, MagicalDefenseRandom),
		RandomizeValueWithRange(ItemInfo.ItemStats.BlockingMagicalDefense, BlockingMagicalDefenseRandom),
		RandomizeValueWithRange(ItemInfo.ItemStats.MaxHealth, MaxHealthRandom),
		ItemInfo.ItemStats.MaxPotionCharge,
		ItemInfo.ItemStats.MovementSpeed == 1 ?
			ItemInfo.ItemStats.MovementSpeed :
			static_cast<int>(RandomizeFValueWithRange(ItemInfo.ItemStats.MovementSpeed, MovementSpeedRandom))
	};
}

void AEfhorisGameMode::StoreItem(APickableItem* Item)
{
	GetGameState<AEfhorisGameState>()->StoreItem(Item);
}

void AEfhorisGameMode::StoreItemInfo(FItemInfo ItemInfo)
{
	GetGameState<AEfhorisGameState>()->StoreItemInfo(ItemInfo);
}

void AEfhorisGameMode::RemoveItemInfo(FItemInfo Item)
{
	GetGameState<AEfhorisGameState>()->RemoveItemInfo(Item);
}

void AEfhorisGameMode::EquipKitToPlayer(APlayerCharacter* Character, FItemInfo ItemInfo)
{
	if(const AEfhorisGameState* EfhorisGameState = GetGameState<AEfhorisGameState>(); ensure(IsValid(EfhorisGameState)))
	{
		if(EfhorisGameState->ContainsItem(ItemInfo))
		{
			if (ItemInfo.ItemType == EItemType::Trash)
				return;

			//J'equipe le joueur
			if (ItemInfo.ItemType == EItemType::Weapon)
			{
				if(Character->HasWeaponKit())
				{
					const FItemInfo PlayerOldWeaponKitInfo = Character->GetWeaponKitInfo();
					StoreItemInfo(PlayerOldWeaponKitInfo);
					Character->EquipWeaponKit(ItemInfo);
				}
				else
				{
					Character->EquipWeaponKit(ItemInfo);
				}
			}

			if (ItemInfo.ItemType == EItemType::Armor)
			{
				if (Character->HasArmorKit())
				{
					const FItemInfo PlayerOldArmorKitInfo = Character->GetArmorKitInfo();
					StoreItemInfo(PlayerOldArmorKitInfo);
					Character->EquipArmorKit(ItemInfo);
				}
				else
				{
					Character->EquipArmorKit(ItemInfo);
				}
			}

			if (ItemInfo.ItemType == EItemType::Gloves)
			{
				if (Character->HasGloves())
				{
					const FItemInfo PlayerOldGlovesInfo = Character->GetGlovesInfo();
					StoreItemInfo(PlayerOldGlovesInfo);
					Character->EquipGloves(ItemInfo);
				}
				else
				{
					Character->EquipGloves(ItemInfo);
				}
			}

			if (ItemInfo.ItemType == EItemType::Belt)
			{
				if (Character->HasBelt())
				{
					const FItemInfo PlayerOldBeltInfo = Character->GetBeltInfo();
					StoreItemInfo(PlayerOldBeltInfo);
					Character->EquipBelt(ItemInfo);
				}
				else
				{
					Character->EquipBelt(ItemInfo);
				}
			}

			if (ItemInfo.ItemType == EItemType::Ring)
			{
				if (Character->HasRing())
				{
					const FItemInfo PlayerOldRingInfo = Character->GetRingInfo();
					StoreItemInfo(PlayerOldRingInfo);
					Character->EquipRing(ItemInfo);
				}
				else
				{
					Character->EquipRing(ItemInfo);
				}
			}

			if (ItemInfo.ItemType == EItemType::Necklace)
			{
				if (Character->HasNecklace())
				{
					const FItemInfo PlayerOldNecklaceInfo = Character->GetNecklaceInfo();
					StoreItemInfo(PlayerOldNecklaceInfo);
					Character->EquipNecklace(ItemInfo);
				}
				else
				{
					Character->EquipNecklace(ItemInfo);
				}
			}

			if (ItemInfo.ItemType == EItemType::Earrings)
			{
				if (Character->HasEarrings())
				{
					const FItemInfo PlayerOldEarringsInfo = Character->GetEarringsInfo();
					StoreItemInfo(PlayerOldEarringsInfo);
					Character->EquipEarrings(ItemInfo);
				}
				else
				{
					Character->EquipEarrings(ItemInfo);
				}
			}

			RemoveItemInfo(ItemInfo);
		}
	}
}

void AEfhorisGameMode::UnequipKitFromPlayer(APlayerCharacter* Character, FItemInfo ItemInfo)
{
	//J'equipe le joueur
	if (ItemInfo.ItemType == EItemType::Weapon)
	{
		if (Character->HasWeaponKit())
		{
			const FItemInfo PlayerOldWeaponKitInfo = Character->GetWeaponKitInfo();
			StoreItemInfo(PlayerOldWeaponKitInfo);
			Character->UnequipWeaponKit(false);
		}
	}

	if (ItemInfo.ItemType == EItemType::Armor)
	{
		if (Character->HasArmorKit())
		{
			const FItemInfo PlayerOldArmorKitInfo = Character->GetArmorKitInfo();
			StoreItemInfo(PlayerOldArmorKitInfo);
			Character->UnequipArmorKit();
		}
	}

	if (ItemInfo.ItemType == EItemType::Gloves)
	{
		if (Character->HasGloves())
		{
			const FItemInfo PlayerOldGlovesInfo = Character->GetGlovesInfo();
			StoreItemInfo(PlayerOldGlovesInfo);
			Character->UnequipGloves();
		}
	}

	if (ItemInfo.ItemType == EItemType::Belt)
	{
		if (Character->HasBelt())
		{
			const FItemInfo PlayerOldBeltInfo = Character->GetBeltInfo();
			StoreItemInfo(PlayerOldBeltInfo);
			Character->UnequipBelt();
		}
	}

	if (ItemInfo.ItemType == EItemType::Ring)
	{
		if (Character->HasRing())
		{
			const FItemInfo PlayerOldRingInfo = Character->GetRingInfo();
			StoreItemInfo(PlayerOldRingInfo);
			Character->UnequipRing();
		}
	}

	if (ItemInfo.ItemType == EItemType::Necklace)
	{
		if (Character->HasNecklace())
		{
			const FItemInfo PlayerOldNecklaceInfo = Character->GetNecklaceInfo();
			StoreItemInfo(PlayerOldNecklaceInfo);
			Character->UnequipNecklace();
		}
	}

	if (ItemInfo.ItemType == EItemType::Earrings)
	{
		if (Character->HasEarrings())
		{
			const FItemInfo PlayerOldEarringsInfo = Character->GetEarringsInfo();
			StoreItemInfo(PlayerOldEarringsInfo);
			Character->UnequipEarrings();
		}
	}
}

void AEfhorisGameMode::SetSmithItemToPlayer(APlayerCharacter* Character, FItemInfo ItemInfo)
{
	if(ItemInfo.ItemType != EItemType::Trash)
	{
		if (GetGameState<AEfhorisGameState>()->ContainsItem(ItemInfo))
		{
			if (Character->HasSmithItem())
			{
				//Add back the item in the shared inventory
				StoreItemInfo(Character->GetSmithItem());
			}
			//Remove item from inventory
			RemoveItemInfo(ItemInfo);
			//Set new item as smithItem
			Character->SetSmithItem(ItemInfo);
			Character->SetHasSmithItem(true);
		}
	}
}

void AEfhorisGameMode::RemoveSmithItemFromPlayer(APlayerCharacter* Character)
{
	if (Character->HasSmithItem())
	{
		//Add back the item in the shared inventory
		StoreItemInfo(Character->GetSmithItem());
	}
	Character->SetHasSmithItem(false);
}

void AEfhorisGameMode::SmithPlayerItem(APlayerCharacter* Player, int LevelOffset)
{
	if (Player->HasSmithItem()) 
	{
		if (AEfhorisGameState* EfhorisGameState = GetGameState<AEfhorisGameState>(); ensure(IsValid(EfhorisGameState)))
		{
			const FItemInfo Item = Player->GetSmithItem();
			const int Money = EfhorisGameState->GetMoney() - EfhorisGameState->GetSmithCost(Item);
			if (Money >= 0)
			{
				if (Item.ItemType == EItemType::Weapon)
				{
					const FWeaponKitRow* Row = GetGameInstance<UEfhorisGameInstance>()->GetWeaponKitsDataTable()->FindRow<FWeaponKitRow>(Item.RowName, {});

					FItemInfo SmithedWeapon = GenerateWeaponKitInfo(Item.RowName, *Row, Item.Level + LevelOffset, true);

					StoreItemInfo(SmithedWeapon);
				}
				else if (Item.ItemType == EItemType::Armor)
				{
					const FArmorKitRow* Row = GetGameInstance<UEfhorisGameInstance>()->GetArmorKitsDataTable()->FindRow<FArmorKitRow>(Item.RowName, {});

					FItemInfo SmithedArmor = GenerateArmorKitInfo(Item.RowName, *Row, Item.Level + LevelOffset, true);

					StoreItemInfo(SmithedArmor);

				}
				else if (Item.ItemType == EItemType::Belt)
				{
					const FBeltRow* Row = GetGameInstance<UEfhorisGameInstance>()->GetBeltsDataTable()->FindRow<FBeltRow>(Item.RowName, {});

					FItemInfo SmithedBelt = GenerateBeltInfo(Item.RowName, *Row, Item.Level + LevelOffset, true);

					StoreItemInfo(SmithedBelt);

				}
				else if (Item.ItemType == EItemType::Gloves)
				{
					const FGlovesRow* Row = GetGameInstance<UEfhorisGameInstance>()->GetGlovesDataTable()->FindRow<FGlovesRow>(Item.RowName, {});

					FItemInfo SmithedGloves = GenerateGlovesInfo(Item.RowName, *Row, Item.Level + LevelOffset, true);

					StoreItemInfo(SmithedGloves);

				}
				else if (Item.ItemType == EItemType::Necklace)
				{
					const FNecklaceRow* Row = GetGameInstance<UEfhorisGameInstance>()->GetNecklacesDataTable()->FindRow<FNecklaceRow>(Item.RowName, {});

					FItemInfo SmithedNecklace = GenerateNecklaceInfo(Item.RowName, *Row, Item.Level + LevelOffset, true);

					StoreItemInfo(SmithedNecklace);

				}
				else if (Item.ItemType == EItemType::Earrings)
				{
					const FEarringsRow* Row = GetGameInstance<UEfhorisGameInstance>()->GetEarringsDataTable()->FindRow<FEarringsRow>(Item.RowName, {});

					FItemInfo SmithedEarrings = GenerateEarringsInfo(Item.RowName, *Row, Item.Level + LevelOffset, true);

					StoreItemInfo(SmithedEarrings);

				}
				else if (Item.ItemType == EItemType::Ring)
				{
					const FRingRow* Row = GetGameInstance<UEfhorisGameInstance>()->GetRingsDataTable()->FindRow<FRingRow>(Item.RowName, {});

					FItemInfo SmithedRing = GenerateRingInfo(Item.RowName, *Row, Item.Level + LevelOffset, true);

					StoreItemInfo(SmithedRing);

				}
				Player->SetHasSmithItem(false);
				RemoveSmithItemFromPlayer(Player);
				EfhorisGameState->SetMoney(Money);

				if (AEfhorisPlayerState* PlayerState = Player->GetPlayerState<AEfhorisPlayerState>(); IsValid(PlayerState))
					PlayerState->IncreaseStatistic(EUserScoreboardStatistic::ItemsUpgraded, 1);
			}
		}
	}
}

void AEfhorisGameMode::TryHealAllPlayers()
{
	if (AEfhorisGameState* EfhorisGameState = GetGameState<AEfhorisGameState>())
	{
		const int Money = EfhorisGameState->GetMoney() - EfhorisGameState->GetHealAllPlayersCost();
		if (Money >= 0)
		{
			for (auto& PlayerState : EfhorisGameState->PlayerArray)
			{
				APawn* Pawn = PlayerState->GetPawn();
				APlayerCharacter* Player = Cast<APlayerCharacter>(Pawn);

				Player->Regen();
			}
			EfhorisGameState->SetMoney(Money);
			EfhorisGameState->DoubleHealCost();
		}
	}
}

void AEfhorisGameMode::TryBuyItem(AMerchant* Merchant, FItemInfo ItemInfo, const APlayerCharacter* Player)
{
	if (AEfhorisGameState* EfhorisGameState = GetGameState<AEfhorisGameState>())
	{
		const int Money = EfhorisGameState->GetMoney() - ItemInfo.BuyValue;

		if (Money >= 0)
		{
			StoreItemInfo(FItemInfo::Copy(ItemInfo));
			EfhorisGameState->SetMoney(Money);

			if (!Merchant->IsInfinite())
				Merchant->RemoveItemFromStock(ItemInfo);

			if (AEfhorisPlayerState* PlayerState = Player->GetPlayerState<AEfhorisPlayerState>(); IsValid(PlayerState))
				PlayerState->IncreaseStatistic(EUserScoreboardStatistic::ItemsBought, 1);
		}
	}
}

void AEfhorisGameMode::TrySellItem(AMerchant* Merchant, FItemInfo ItemInfo, const APlayerCharacter* Player)
{
	if (AEfhorisGameState* EfhorisGameState = GetGameState<AEfhorisGameState>())
	{
		RemoveItemInfo(ItemInfo);

		int SellValue = ItemInfo.SellValue;

		if (ItemInfo.ItemType == EItemType::Trash)
			SellValue *= ItemInfo.Stack;

		EfhorisGameState->SetMoney(EfhorisGameState->GetMoney() + SellValue);

		if (AEfhorisPlayerState* PlayerState = Player->GetPlayerState<AEfhorisPlayerState>(); IsValid(PlayerState))
			PlayerState->IncreaseStatistic(EUserScoreboardStatistic::ItemsSold, 1);
	}
}

APickableItem* AEfhorisGameMode::SpawnArmorKit(const FName& RowName, const FTransform& Transform, int Level, bool bRandomizeStats)
{
	const FArmorKitRow* Row = GetGameInstance<UEfhorisGameInstance>()->GetArmorKitsDataTable()->FindRow<FArmorKitRow>(RowName, {});

	FActorSpawnParameters SpawnParameters{};
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APickableItem* ArmorKit = SpawnActor<APickableItem>(Row->LootActor, Transform, SpawnParameters);
	if (!IsValid(ArmorKit)) return nullptr;

	ArmorKit->SetItemInfo(GenerateArmorKitInfo(RowName, *Row, Level, bRandomizeStats));
	ArmorKit->SetIsPickable(true);
	DroppedItems.Add(ArmorKit);
	return ArmorKit;
}

APickableItem* AEfhorisGameMode::SpawnTrashItem(const FName& RowName, const FTransform& Transform, int Level)
{
	const FTrashItemRow* Row = GetGameInstance<UEfhorisGameInstance>()->GetTrashItemsDataTable()->FindRow<FTrashItemRow>(RowName, {});

	FActorSpawnParameters SpawnParameters{};
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APickableItem* TrashItem = SpawnActor<APickableItem>(Row->LootActor, Transform, SpawnParameters);
	if (!IsValid(TrashItem)) return nullptr;

	TrashItem->SetItemInfo(GenerateTrashItemInfo(RowName, *Row, Level));
	TrashItem->SetIsPickable(true);
	DroppedItems.Add(TrashItem);
	return TrashItem;
}

APickableItem* AEfhorisGameMode::SpawnWeaponKit(const FName& RowName, const FTransform& Transform, int Level, bool bRandomizeStats)
{
	const FWeaponKitRow* Row = GetGameInstance<UEfhorisGameInstance>()->GetWeaponKitsDataTable()->FindRow<FWeaponKitRow>(RowName, {});

	FActorSpawnParameters SpawnParameters{};
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APickableItem* WeaponKit = SpawnActor<APickableItem>(Row->LootActor, Transform, SpawnParameters);
	if (!IsValid(WeaponKit)) return nullptr;

	WeaponKit->SetItemInfo(GenerateWeaponKitInfo(RowName, *Row, Level, bRandomizeStats));
	WeaponKit->SetIsPickable(true);
	DroppedItems.Add(WeaponKit);
	return WeaponKit;
}

AHealSeed* AEfhorisGameMode::SpawnHealSeed(const FTransform& Transform)
{
	FActorSpawnParameters SpawnParameters{};
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AHealSeed* HealSeed = SpawnActor<AHealSeed>(HealSeedClass, Transform, SpawnParameters);
	DroppedItems.Add(HealSeed);
	return HealSeed;
}

APickableItem* AEfhorisGameMode::SpawnBelt(const FName& RowName, const FTransform& Transform, int Level,
	bool bRandomizeStats)
{
	const FBeltRow* Row = GetGameInstance<UEfhorisGameInstance>()->GetBeltsDataTable()->FindRow<FBeltRow>(RowName, {});

	FActorSpawnParameters SpawnParameters{};
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APickableItem* Belt = SpawnActor<APickableItem>(Row->LootActor, Transform, SpawnParameters);
	if (!IsValid(Belt)) return nullptr;

	Belt->SetItemInfo(GenerateBeltInfo(RowName, *Row, Level, bRandomizeStats));
	Belt->SetIsPickable(true);
	DroppedItems.Add(Belt);
	return Belt;
}

APickableItem* AEfhorisGameMode::SpawnGloves(const FName& RowName, const FTransform& Transform, int Level,
	bool bRandomizeStats)
{
	const FGlovesRow* Row = GetGameInstance<UEfhorisGameInstance>()->GetGlovesDataTable()->FindRow<FGlovesRow>(RowName, {});

	FActorSpawnParameters SpawnParameters{};
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APickableItem* Gloves = SpawnActor<APickableItem>(Row->LootActor, Transform, SpawnParameters);
	if (!IsValid(Gloves)) return nullptr;

	Gloves->SetItemInfo(GenerateGlovesInfo(RowName, *Row, Level, bRandomizeStats));
	Gloves->SetIsPickable(true);
	DroppedItems.Add(Gloves);
	return Gloves;
}

APickableItem* AEfhorisGameMode::SpawnNecklace(const FName& RowName, const FTransform& Transform, int Level,
	bool bRandomizeStats)
{
	const FNecklaceRow* Row = GetGameInstance<UEfhorisGameInstance>()->GetNecklacesDataTable()->FindRow<FNecklaceRow>(RowName, {});

	FActorSpawnParameters SpawnParameters{};
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APickableItem* Necklace = SpawnActor<APickableItem>(Row->LootActor, Transform, SpawnParameters);
	if (!IsValid(Necklace)) return nullptr;

	Necklace->SetItemInfo(GenerateNecklaceInfo(RowName, *Row, Level, bRandomizeStats));
	Necklace->SetIsPickable(true);
	DroppedItems.Add(Necklace);
	return Necklace;
}

APickableItem* AEfhorisGameMode::SpawnEarrings(const FName& RowName, const FTransform& Transform, int Level,
	bool bRandomizeStats)
{
	const FEarringsRow* Row = GetGameInstance<UEfhorisGameInstance>()->GetEarringsDataTable()->FindRow<FEarringsRow>(RowName, {});

	FActorSpawnParameters SpawnParameters{};
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APickableItem* Earrings = SpawnActor<APickableItem>(Row->LootActor, Transform, SpawnParameters);
	if (!IsValid(Earrings)) return nullptr;

	Earrings->SetItemInfo(GenerateEarringsInfo(RowName, *Row, Level, bRandomizeStats));
	Earrings->SetIsPickable(true);
	DroppedItems.Add(Earrings);
	return Earrings;
}

APickableItem* AEfhorisGameMode::SpawnRing(const FName& RowName, const FTransform& Transform, int Level,
	bool bRandomizeStats)
{
	const FRingRow* Row = GetGameInstance<UEfhorisGameInstance>()->GetRingsDataTable()->FindRow<FRingRow>(RowName, {});

	FActorSpawnParameters SpawnParameters{};
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APickableItem* Ring = SpawnActor<APickableItem>(Row->LootActor, Transform, SpawnParameters);
	if (!IsValid(Ring)) return nullptr;

	Ring->SetItemInfo(GenerateRingInfo(RowName, *Row, Level, bRandomizeStats));
	Ring->SetIsPickable(true);
	DroppedItems.Add(Ring);
	return Ring;
}

FItemInfo AEfhorisGameMode::GenerateArmorKitInfo(const FName& RowName, TOptional<FArmorKitRow> Row, int Level, bool bRandomizeStats) const
{
	if (Level < 1) Level = 1;

	if (!Row.IsSet())
		Row = *GetGameInstance<UEfhorisGameInstance>()->GetArmorKitsDataTable()->FindRow<FArmorKitRow>(RowName, {});

	FItemInfo Infos{};
	Infos.RowName = RowName;
	Infos.ItemType = EItemType::Armor;
	Infos.Level = Level;

	Infos.BuyValue = Row->BaseBuyValue;
	Infos.SellValue = Row->BaseSellValue;
	Infos.ItemStats = Row->BaseItemStats;

	if (bRandomizeStats)
		RandomizeItemInfo(Infos);

	ScaleItemInfo(Infos);

	return Infos;
}

FItemInfo AEfhorisGameMode::GenerateTrashItemInfo(const FName& RowName, TOptional<FTrashItemRow> Row, int Level) const
{
	if (Level < 1) Level = 1;

	if (!Row.IsSet())
		Row = *GetGameInstance<UEfhorisGameInstance>()->GetTrashItemsDataTable()->FindRow<FTrashItemRow>(RowName, {});

	FItemInfo Infos{};
	Infos.RowName = RowName;
	Infos.ItemType = EItemType::Trash;
	Infos.Level = Level;

	Infos.SellValue = Row->BaseSellValue;

	ScaleItemInfo(Infos);

	return Infos;
}

FItemInfo AEfhorisGameMode::GenerateWeaponKitInfo(const FName& RowName, TOptional<FWeaponKitRow> Row, int Level, bool bRandomizeStats) const
{
	if (Level < 1) Level = 1;

	if (!Row.IsSet())
		Row = *GetGameInstance<UEfhorisGameInstance>()->GetWeaponKitsDataTable()->FindRow<FWeaponKitRow>(RowName, {});

	FItemInfo Infos{};
	Infos.RowName = RowName;
	Infos.ItemType = EItemType::Weapon;
	Infos.Level = Level;

	Infos.BuyValue = Row->BaseBuyValue;
	Infos.SellValue = Row->BaseSellValue;
	Infos.ItemStats = Row->BaseItemStats;

	if (bRandomizeStats)
		RandomizeItemInfo(Infos);

	ScaleItemInfo(Infos);

	// TODO if we have more than 2 abilities for a single kit then we should add some random here
	if (Row->PrimaryAbilities.Num()) {
		Infos.Ability1 = Row->PrimaryAbilities[0];
		Infos.Ability1ClassPath = Infos.Ability1->GetClassPathName();
	}

	if (Row->SecondaryAbilities.Num()) {
		Infos.Ability2 = Row->SecondaryAbilities[0];
		Infos.Ability2ClassPath = Infos.Ability2->GetClassPathName();
	}

	return Infos;
}

FItemInfo AEfhorisGameMode::GenerateGlovesInfo(const FName& RowName, TOptional<FGlovesRow> Row, int Level,
	bool bRandomizeStats) const
{
	if (Level < 1) Level = 1;

	if (!Row.IsSet())
		Row = *GetGameInstance<UEfhorisGameInstance>()->GetGlovesDataTable()->FindRow<FGlovesRow>(RowName, {});

	FItemInfo Infos{};
	Infos.RowName = RowName;
	Infos.ItemType = EItemType::Gloves;
	Infos.Level = Level;

	Infos.BuyValue = Row->BaseBuyValue;
	Infos.SellValue = Row->BaseSellValue;
	Infos.ItemStats = Row->BaseItemStats;

	if (bRandomizeStats)
		RandomizeItemInfo(Infos);

	ScaleItemInfo(Infos);

	return Infos;
}

FItemInfo AEfhorisGameMode::GenerateBeltInfo(const FName& RowName, TOptional<FBeltRow> Row, int Level,
	bool bRandomizeStats) const
{
	if (Level < 1) Level = 1;

	if (!Row.IsSet())
		Row = *GetGameInstance<UEfhorisGameInstance>()->GetBeltsDataTable()->FindRow<FBeltRow>(RowName, {});

	FItemInfo Infos{};
	Infos.RowName = RowName;
	Infos.ItemType = EItemType::Belt;
	Infos.Level = Level;

	Infos.BuyValue = Row->BaseBuyValue;
	Infos.SellValue = Row->BaseSellValue;
	Infos.ItemStats = Row->BaseItemStats;

	if (bRandomizeStats)
		RandomizeItemInfo(Infos);

	ScaleItemInfo(Infos);

	return Infos;
}

FItemInfo AEfhorisGameMode::GenerateRingInfo(const FName& RowName, TOptional<FRingRow> Row, int Level,
	bool bRandomizeStats) const
{
	if (Level < 1) Level = 1;

	if (!Row.IsSet())
		Row = *GetGameInstance<UEfhorisGameInstance>()->GetRingsDataTable()->FindRow<FRingRow>(RowName, {});

	FItemInfo Infos{};
	Infos.RowName = RowName;
	Infos.ItemType = EItemType::Ring;
	Infos.Level = Level;

	Infos.BuyValue = Row->BaseBuyValue;
	Infos.SellValue = Row->BaseSellValue;
	Infos.ItemStats = Row->BaseItemStats;

	if (bRandomizeStats)
		RandomizeItemInfo(Infos);

	ScaleItemInfo(Infos);

	return Infos;
}

FItemInfo AEfhorisGameMode::GenerateNecklaceInfo(const FName& RowName, TOptional<FNecklaceRow> Row, int Level,
	bool bRandomizeStats) const
{
	if (Level < 1) Level = 1;

	if (!Row.IsSet())
		Row = *GetGameInstance<UEfhorisGameInstance>()->GetNecklacesDataTable()->FindRow<FNecklaceRow>(RowName, {});

	FItemInfo Infos{};
	Infos.RowName = RowName;
	Infos.ItemType = EItemType::Necklace;
	Infos.Level = Level;

	Infos.BuyValue = Row->BaseBuyValue;
	Infos.SellValue = Row->BaseSellValue;
	Infos.ItemStats = Row->BaseItemStats;

	if (bRandomizeStats)
		RandomizeItemInfo(Infos);

	ScaleItemInfo(Infos);

	return Infos;
}

FItemInfo AEfhorisGameMode::GenerateEarringsInfo(const FName& RowName, TOptional<FEarringsRow> Row, int Level,
	bool bRandomizeStats) const
{
	if (Level < 1) Level = 1;

	if (!Row.IsSet())
		Row = *GetGameInstance<UEfhorisGameInstance>()->GetEarringsDataTable()->FindRow<FEarringsRow>(RowName, {});

	FItemInfo Infos{};
	Infos.RowName = RowName;
	Infos.ItemType = EItemType::Earrings;
	Infos.Level = Level;

	Infos.BuyValue = Row->BaseBuyValue;
	Infos.SellValue = Row->BaseSellValue;
	Infos.ItemStats = Row->BaseItemStats;

	if (bRandomizeStats)
		RandomizeItemInfo(Infos);

	ScaleItemInfo(Infos);

	return Infos;
}

void AEfhorisGameMode::WinGame()
{
	if (auto EfhorisGameInstance = GetGameInstance<UEfhorisGameInstance>(); IsValid(EfhorisGameInstance))
	{
		if (const AEfhorisGameState* EfhorisGameState = Cast<AEfhorisGameState>(GameState); IsValid(EfhorisGameState) && EfhorisGameState->GetCurrentLevel() >= 3) {
			for (const APlayerState* State : GameState->PlayerArray)
			{
				if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(State->GetPlayerController()); IsValid(PlayerController))
				{
					PlayerController->Client_Win();
				}
			}
		}
		else {
			//StopAllMusic();
			PlayMusic(EfhorisGameInstance->Boss1Defeated);
			PlaySequence(EfhorisGameInstance->GetEndBossSequence(), true);
			RebuildLevel(true);
		}
	}
}

void AEfhorisGameMode::TryLoseGame()
{
	bool bShouldLose = true;

	for (const APlayerState* State : GameState->PlayerArray)
	{
		if (APlayerCharacter* Player = Cast<APlayerCharacter>(State->GetPawn()); IsValid(Player))
		{
			if (!Player->GetMercenaries().IsEmpty() || Player->IsAliveTag())
				bShouldLose = false;
		}
	}

	if (bShouldLose) {
		if (const AEfhorisGameState* EfhorisGameState = Cast<AEfhorisGameState>(GameState); IsValid(EfhorisGameState)) {
			for (APlayerState* State : GameState->PlayerArray) {
				if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(State->GetPlayerController()); IsValid(PlayerController)) {
					if (PlayerController->GetPawn()->IsA<APlayerCharacter>())
						if (AEfhorisPlayerState* EfhorisPlayerState = Cast<AEfhorisPlayerState>(State); IsValid(EfhorisPlayerState))
							EfhorisPlayerState->IncreaseStatistic(EUserScoreboardStatistic::Deaths, 1);

					PlayerController->Client_Lose();
				}
			}
		}
	}
}

void AEfhorisGameMode::ReloadGame()
{
	RebuildLevel(false, false);
}

void AEfhorisGameMode::CallInventory(APlayerCharacter* PlayerCharacter) const
{
	if(const AEfhorisGameState* EfhorisGameState = GetGameState<AEfhorisGameState>(); ensure(IsValid(EfhorisGameState)))
	{
		EfhorisGameState->GetInventory()->SetTarget(PlayerCharacter);
	}
}

void AEfhorisGameMode::AddPawnToDeleteOnLevelReset(APawn* Pawn)
{
	if (IsValid(Pawn))
		PawnsToDeleteOnReset.Add(Pawn);
}

void AEfhorisGameMode::RebuildLevel(bool bGameWon, bool bShouldSave)
{
	if (AEfhorisGameState* EfhorisGameState = GetGameState<AEfhorisGameState>(); ensure(IsValid(EfhorisGameState)))
	{
		EfhorisGameState->SetGameEnded(true);

		AAIManager* Manager = EfhorisGameState->GetAIManager();
		if (IsValid(Manager)) {
			for (ADragonBoss* Boss : Manager->GetBosses()) {
				Boss->HideHealthBar();
			}
			Manager->ClearUnits();
		}

		// Validate & generate contracts
		if (bGameWon)
			CheckValidateContracts();

		GenerateContracts();

		if (EfhorisGameState->IsLevelGeneratorValid())
		{
			EfhorisGameState->GetAIDirector()->ResetDirector();

			EfhorisGameState->GetLevelGenerator()->ResetLevel(bGameWon);

			EfhorisGameState->GenerateLayoutMinimap();

			EfhorisGameState->Authority_ResetLevelTimer();

			UE_LOG(LogTemp, Warning, TEXT("RebuildLevel, game is won ? %i"), bGameWon);
			int i = 0;

			for (auto& PlayerState : EfhorisGameState->PlayerArray)
			{
				APawn* Pawn = PlayerState->GetPawn();
				APlayerCharacter* Player = Cast<APlayerCharacter>(Pawn);
			
				if (!IsValid(Player) || !Player->IsAliveHealth())
				{
					UE_LOG(LogTemp, Warning, TEXT("\t Player°%d is dead or unvalid "), i++);
					AEfhorisPlayerController* Controller = Pawn->GetController<AEfhorisPlayerController>();
					AddPawnToDeleteOnLevelReset(Pawn);

					FActorSpawnParameters ActorSpawnParameters;
					ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					APlayerCharacter* NewPlayer = SpawnActor<APlayerCharacter>(DefaultPawnClass, FTransform(FVector(0, 0, 200)), ActorSpawnParameters);

					Controller->Possess(NewPlayer);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("\t Player°%d is alive and is tp to start"), i++);
					AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(Player->GetController());
					if (IsValid(PlayerController)) {
						PlayerController->Authority_TeleportPlayer(FVector(0, 0, 200));
						Player->Regen();
						
						const FGameplayTagContainer TagCancel {
							FGameplayTag::RequestGameplayTag(FName("Character.AbilityEvent.CancelAllActions"))
						};
						Player->GetAbilitySystemComponent()->TryActivateAbilitiesByTag(TagCancel);
						PlayerController->Client_ForceCloseAllMenus(false);
					}
				}
			}

			for (APawn* Pawn : PawnsToDeleteOnReset)
				if (IsValid(Pawn))
					Pawn->Destroy();

			PawnsToDeleteOnReset.Empty();

			EfhorisGameState->GetInventory()->SetActorLocation(FVector{ 0, 0, 20 });
		}
		//Reset InventoryChest
		EfhorisGameState->GetInventory()->ResetRebuild();

		//Reset Heal cost
		EfhorisGameState->ResetHealCost();

		//Update AI Director
		EfhorisGameState->GetAIDirector()->Update(true);

		//Remove dropped items
		for(AActor* Item : DroppedItems) {
			if (IsValid(Item))
				Item->Destroy();
		}
		DroppedItems.Empty();

		// Save game state
		if (bShouldSave)
			SaveGame();

		//StopSaveGameAsync();

		//Reset la game
		EfhorisGameState->SetGameEnded(false);
	}
}

void AEfhorisGameMode::TileChanged(ATileLimit* Limit, FVector2D PlayerPos) const
{
	if (const AEfhorisGameState* EfhorisGameState = GetGameState<AEfhorisGameState>(); ensure(IsValid(EfhorisGameState)))
		if (const ALevelGenerator* LevelGenerator = EfhorisGameState->GetLevelGenerator(); ensure(IsValid(LevelGenerator)))
		{
			// Mettre à jour l'IA Director
			EfhorisGameState->GetAIDirector()->Update(false);

			//Lancer la sauvegarde
			SaveGame(true);
		}
}

void AEfhorisGameMode::AddDamageTaken(double Damage) const
{
	if (AEfhorisGameState* EfhorisGameState = GetGameState<AEfhorisGameState>(); ensure(IsValid(EfhorisGameState))) {
		EfhorisGameState->AddDamageTaken(static_cast<long double>(Damage));
	}
}

void AEfhorisGameMode::RefreshHealthPanel(const TArray<FPlayerInfo>& Descriptions)
{
	if (AEfhorisGameState* EfhorisGameState = GetGameState<AEfhorisGameState>(); ensure(IsValid(EfhorisGameState))) {
		for(auto& Player : GameState->PlayerArray)
		{
			if(AEfhorisPlayerState* PlayerState = Cast<AEfhorisPlayerState>(Player);IsValid(PlayerState))
				PlayerState->Client_RefreshHealthPanel(Descriptions);
		}
	}
}

void AEfhorisGameMode::RefreshMapNicknames(const TArray<FPlayerInfo>& Descriptions) {
	if (AEfhorisGameState* EfhorisGameState = GetGameState<AEfhorisGameState>(); ensure(IsValid(EfhorisGameState))) {
		for (auto& Player : GameState->PlayerArray)
		{
			if (AEfhorisPlayerState* PlayerState = Cast<AEfhorisPlayerState>(Player); IsValid(PlayerState))
				PlayerState->Client_RefreshMapNicknames(Descriptions);
		}
	}
}

void AEfhorisGameMode::StartBoss()
{
	const ADragonBoss* FirstBoss = Cast<AEfhorisGameState>(GameState)->GetAIManager()->GetBosses()[0];
	const FVector BossForward = FirstBoss->GetActorForwardVector();

	const FVector TPPos = FirstBoss->GetInitialLocation() + BossForward * 2000.f + FVector(0,0,100);

	for (const auto& PlayerState : GameState->PlayerArray)
	{
		APawn* Pawn = PlayerState->GetPawn();
		APlayerCharacter* Player = Cast<APlayerCharacter>(Pawn);

		if (IsValid(Player)) {
			if (Player->IsAliveHealth()) {
				Player->GetController<AEfhorisPlayerController>()->Authority_TeleportPlayer(TPPos);
			}
			else {
				Player->MercenariesFindNewMaster();
			}
			for (const auto Mercenary : Player->GetMercenaries())
			{
				if(IsValid(Mercenary))
					Mercenary->Authority_Teleport(TPPos);
			}
		}
	}
	//Lancer cinematique
	if (const auto EfhorisGameInstance = GetGameInstance<UEfhorisGameInstance>(); IsValid(EfhorisGameInstance))
	{
		//StopAllMusic();
		PlaySequence(EfhorisGameInstance->GetStartBossSequence());
		PlayMusic(EfhorisGameInstance->Boss1Start);
		SetMusicState(FName("SG_BossFight"), FName("Phase_01"));
	}

	if (auto EGameState = Cast<AEfhorisGameState>(GameState); IsValid(EGameState))
		if (auto AImanager = EGameState->GetAIManager(); IsValid(AImanager))
			AImanager->KillEnemies();
}

void AEfhorisGameMode::EnterBossCombat()
{
	for(auto Boss : GetGameState<AEfhorisGameState>()->GetAIManager()->GetBosses())
	{
		Boss->SetIsInCombat(true);
		Boss->ShowHealthBar();
	}
}

void AEfhorisGameMode::PlaySequence(ULevelSequence* SequenceToPlay, bool bShouldPlayStartSequenceAfter)
{
	for (const auto& PlayerState : GameState->PlayerArray)
	{
		if (AEfhorisPlayerController* Controller = Cast<AEfhorisPlayerController>(PlayerState->GetPlayerController()); IsValid(Controller)) {
			Controller->Client_PlaySequence(SequenceToPlay, bShouldPlayStartSequenceAfter);
		}
	}
}

void AEfhorisGameMode::PlayMusic(UAkAudioEvent* MusicToPlay)
{
	for (const auto& PlayerState : GameState->PlayerArray)
	{
		if (AEfhorisPlayerController* Controller = Cast<AEfhorisPlayerController>(PlayerState->GetPlayerController()); IsValid(Controller)) {
			Controller->Client_PlayEvent(MusicToPlay);
		}
	}
}

void AEfhorisGameMode::StopAllMusic()
{
	for (const auto& PlayerState : GameState->PlayerArray)
	{
		if (AEfhorisPlayerController* Controller = Cast<AEfhorisPlayerController>(PlayerState->GetPlayerController()); IsValid(Controller)) {
			Controller->Client_StopAllMusic();
		}
	}
}

void AEfhorisGameMode::SetMusicState(FName StateGroup, FName NewState)
{
	for (const auto& PlayerState : GameState->PlayerArray)
	{
		if (AEfhorisPlayerController* Controller = Cast<AEfhorisPlayerController>(PlayerState->GetPlayerController()); IsValid(Controller)) {
			Controller->Client_SetMusicState(StateGroup, NewState);
		}
	}
}

void AEfhorisGameMode::SaveGame(bool bIsAsynch /* = false */) const
{
	USaveSubsystem* Subsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();

	Subsystem->SaveGame(bIsAsynch);
}

void AEfhorisGameMode::GenerateContracts()
{
	if (AEfhorisGameState* EfhorisGameState = GetGameState<AEfhorisGameState>(); IsValid(EfhorisGameState))
	{
		GenerateAccumulationContract(EfhorisGameState);
		GenerateExplorationContract(EfhorisGameState);
		GenerateFailableContract(EfhorisGameState);
	}
}

void AEfhorisGameMode::GenerateAccumulationContract(AEfhorisGameState* EfhorisGameState)
{
	if (!AccumulationContractTypes.Num()) return;

	AAccumulationContract* Contract = SpawnActor<AAccumulationContract>(AccumulationContractTypes[FMath::RandRange(0, AccumulationContractTypes.Num() - 1)]);
	EfhorisGameState->SetAccumulationContract(Contract);
}

void AEfhorisGameMode::GenerateExplorationContract(AEfhorisGameState* EfhorisGameState)
{
	if (!ExplorationContractTypes.Num()) return;

	AExplorationContract* Contract = SpawnActor<AExplorationContract>(ExplorationContractTypes[FMath::RandRange(0, ExplorationContractTypes.Num() - 1)]);
	EfhorisGameState->SetExplorationContract(Contract);
}

void AEfhorisGameMode::GenerateFailableContract(AEfhorisGameState* EfhorisGameState)
{
	if (!FailableContractTypes.Num()) return;

	AFailableContract* Contract = SpawnActor<AFailableContract>(FailableContractTypes[FMath::RandRange(0, FailableContractTypes.Num() - 1)]);
	EfhorisGameState->SetFailableContract(Contract);
}

void AEfhorisGameMode::CheckValidateContracts()
{
	if (AEfhorisGameState* EfhorisGameState = GetGameState<AEfhorisGameState>(); IsValid(EfhorisGameState))
	{
		int Count = 0;

		if (CheckValidateAccumulationContract(EfhorisGameState)) Count++;
		if (CheckValidateExplorationContract(EfhorisGameState)) Count++;
		if (CheckValidateFailableContract(EfhorisGameState)) Count++;


		for (const auto& PlayerState : GetWorld()->GetGameState()->PlayerArray)
			if (AEfhorisPlayerState* EfhorisPlayerState = Cast<AEfhorisPlayerState>(PlayerState); IsValid(EfhorisPlayerState))
				EfhorisPlayerState->IncreaseStatistic(EUserScoreboardStatistic::TilesExplored, Count);
	}
}

bool AEfhorisGameMode::CheckValidateAccumulationContract(AEfhorisGameState* EfhorisGameState)
{
	const AAccumulationContract* Contract = EfhorisGameState->GetAccumulationContract();

	if (IsValid(Contract) && Contract->GetStatus() == EContractStatus::OnGoing)
	{
		EfhorisGameState->SetMoney(EfhorisGameState->GetMoney() + Contract->GetCount() * Contract->GetReward());
		return true;
	}

	return false;
}

bool AEfhorisGameMode::CheckValidateExplorationContract(AEfhorisGameState* EfhorisGameState)
{
	const AExplorationContract* Contract = EfhorisGameState->GetExplorationContract();

	if (IsValid(Contract) && Contract->GetStatus() == EContractStatus::Succeeded)
	{
		EfhorisGameState->SetMoney(EfhorisGameState->GetMoney() + Contract->GetReward());
		return true;
	}
	return false;
}

bool AEfhorisGameMode::CheckValidateFailableContract(AEfhorisGameState* EfhorisGameState)
{
	const AFailableContract* Contract = EfhorisGameState->GetFailableContract();

	if (IsValid(Contract) && Contract->GetStatus() == EContractStatus::OnGoing)
	{
		EfhorisGameState->SetMoney(EfhorisGameState->GetMoney() + Contract->GetReward());
		return true;
	}
	return false;
}
