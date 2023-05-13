#include "Merchant.h"

#include "Engine/CompositeDataTable.h"
#include "Engine/DataTable.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameInstance.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"
#include "ProjetHiver/Inventory/MerchantItemPool.h"
#include "ProjetHiver/Inventory/MerchantRow.h"
#include "ProjetHiver/Items/ItemRows/ArmorKitRow.h"
#include "ProjetHiver/Items/ItemRows/BeltRow.h"
#include "ProjetHiver/Items/ItemRows/EarringsRow.h"
#include "ProjetHiver/Items/ItemRows/GlovesRow.h"
#include "ProjetHiver/Items/ItemRows/NecklaceRow.h"
#include "ProjetHiver/Items/ItemRows/RingRow.h"
#include "ProjetHiver/Items/ItemRows/WeaponKitRow.h"
#include "ProjetHiver/MapGeneration/LevelGenerator.h"

void AMerchant::BeginPlay()
{
	Super::BeginPlay();

	if (const AEfhorisGameState* EfhorisGameState = GetWorld()->GetGameState<AEfhorisGameState>())
		if (const ALevelGenerator* LevelGenerator = EfhorisGameState->GetLevelGenerator())
			Level = LevelGenerator->CurrentLevel;

	if (HasAuthority())
	{
		GenerateStock();
	}
}

bool AMerchant::IsInteractible(const APlayerCharacter* Player) const {
	return Player->IsAliveTag();
}

void AMerchant::StartInteract(APlayerCharacter* Player)
{
	if (AEfhorisPlayerController* PlayerController = Player->GetController<AEfhorisPlayerController>(); ensure(IsValid(PlayerController)))
	{
		PlayerController->Client_OpenInventoryPanel();
		PlayerController->Client_OpenShopPanel(this);
	}
}

void AMerchant::StopInteract(APlayerCharacter* Player)
{
	if (AEfhorisPlayerController* PlayerController = Player->GetController<AEfhorisPlayerController>(); ensure(IsValid(PlayerController)))
	{
		PlayerController->Client_CloseShopPanel();
	}
}

const FText AMerchant::GetInteractionText() const
{
	return InteractionText;
}

void AMerchant::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMerchant, Stock);
}

bool AMerchant::IsInfinite() const noexcept
{
	return bIsInfinite;
}

void AMerchant::RemoveItemFromStock(FItemInfo ItemInfo)
{
	if (HasAuthority())
		Stock.Remove(ItemInfo);
}

void AMerchant::GenerateStock()
{
	FMerchantRow* Row = GetGameInstance<UEfhorisGameInstance>()->GetMerchantsDataTable()->FindRow<FMerchantRow>(RowName, {});

	if (!Row)
	{
		UE_LOG(LogTemp, Error, TEXT("Row name should be set in merchant %s"), *GetName());
		return;
	}

	bIsInfinite = Row->bIsInfinite;
	
	const AEfhorisGameMode* GameMode = GetWorld()->GetAuthGameMode<AEfhorisGameMode>();

	for (const FMerchantItemPool& ItemPool : Row->ItemPools)
	{
		if (ItemPool.NumberOfItemsToDraw == -1)
		{
			for (const UDataTable* PoolTable : ItemPool.PoolTables)
			{
				TArray<FName> RowNames = PoolTable->GetRowNames();

				if (const FArmorKitRow* RowBool = PoolTable->FindRow<FArmorKitRow>(RowNames[0], "", false); RowBool != nullptr)
				{
					TArray<FArmorKitRow*> Rows{};
					PoolTable->GetAllRows({}, Rows);

					for (int i = 0; i < RowNames.Num(); ++i)
						Stock.Add(GameMode->GenerateArmorKitInfo(RowNames[i], *Rows[i], Level + ItemPool.ItemsLevelDelta, Row->bGenerateRandomStats));
				}
				if(const FWeaponKitRow* RowBool = PoolTable->FindRow<FWeaponKitRow>(RowNames[0], "", false); RowBool != nullptr)
				{
					TArray<FWeaponKitRow*> Rows{};
					PoolTable->GetAllRows({}, Rows);

					for (int i = 0; i < RowNames.Num(); ++i)
						Stock.Add(GameMode->GenerateWeaponKitInfo(RowNames[i], *Rows[i], Level + ItemPool.ItemsLevelDelta, Row->bGenerateRandomStats));
				}
				if (const FGlovesRow* RowBool = PoolTable->FindRow<FGlovesRow>(RowNames[0], "", false); RowBool != nullptr)
				{
					TArray<FGlovesRow*> Rows{};
					PoolTable->GetAllRows({}, Rows);

					for (int i = 0; i < RowNames.Num(); ++i)
						Stock.Add(GameMode->GenerateGlovesInfo(RowNames[i], *Rows[i], Level + ItemPool.ItemsLevelDelta, Row->bGenerateRandomStats));
				}
				if (const FBeltRow* RowBool = PoolTable->FindRow<FBeltRow>(RowNames[0], "", false); RowBool != nullptr)
				{
					TArray<FBeltRow*> Rows{};
					PoolTable->GetAllRows({}, Rows);

					for (int i = 0; i < RowNames.Num(); ++i)
						Stock.Add(GameMode->GenerateBeltInfo(RowNames[i], *Rows[i], Level + ItemPool.ItemsLevelDelta, Row->bGenerateRandomStats));
				}
				if (const FRingRow* RowBool = PoolTable->FindRow<FRingRow>(RowNames[0], "", false); RowBool != nullptr)
				{
					TArray<FRingRow*> Rows{};
					PoolTable->GetAllRows({}, Rows);

					for (int i = 0; i < RowNames.Num(); ++i)
						Stock.Add(GameMode->GenerateRingInfo(RowNames[i], *Rows[i], Level + ItemPool.ItemsLevelDelta, Row->bGenerateRandomStats));
				}
				if (const FNecklaceRow* RowBool = PoolTable->FindRow<FNecklaceRow>(RowNames[0], "", false); RowBool != nullptr)
				{
					TArray<FNecklaceRow*> Rows{};
					PoolTable->GetAllRows({}, Rows);

					for (int i = 0; i < RowNames.Num(); ++i)
						Stock.Add(GameMode->GenerateNecklaceInfo(RowNames[i], *Rows[i], Level + ItemPool.ItemsLevelDelta, Row->bGenerateRandomStats));
				}
				if (const FEarringsRow* RowBool = PoolTable->FindRow<FEarringsRow>(RowNames[0], "", false); RowBool != nullptr)
				{
					TArray<FEarringsRow*> Rows{};
					PoolTable->GetAllRows({}, Rows);

					for (int i = 0; i < RowNames.Num(); ++i)
						Stock.Add(GameMode->GenerateEarringsInfo(RowNames[i], *Rows[i], Level + ItemPool.ItemsLevelDelta, Row->bGenerateRandomStats));
				}
			}
		}
		else
		{
			TArray<int> CumulativeNumberOfItems;
			int TotalNumberOfItems = 0;
			TArray<int> DrawnItems;

			for (const UDataTable* PoolTable : ItemPool.PoolTables)
			{
				int NumberOfItemsInPool = PoolTable->GetRowNames().Num();
				TotalNumberOfItems += NumberOfItemsInPool;

				if (CumulativeNumberOfItems.IsEmpty())
					CumulativeNumberOfItems.Add(NumberOfItemsInPool);
				else
					CumulativeNumberOfItems.Add(CumulativeNumberOfItems.Last() + NumberOfItemsInPool);
			}

			for (int i = 0; i < ItemPool.NumberOfItemsToDraw; ++i)
			{
				int Draw;

				do
				{
					Draw = UKismetMathLibrary::RandomIntegerInRange(0, TotalNumberOfItems - 1);
				} while (DrawnItems.Contains(Draw));

				DrawnItems.Add(Draw);

				for (int j = 0; j < CumulativeNumberOfItems.Num(); ++j)
				{
					if (Draw < CumulativeNumberOfItems[j])
					{
						const UDataTable* TableToDrawFrom = ItemPool.PoolTables[j];
						TArray<FName> RowNames = TableToDrawFrom->GetRowNames();

						//FName RowAssetName = TableToDrawFrom->GetRowStructPathName().GetAssetName();

						if (const FArmorKitRow* RowBool = TableToDrawFrom->FindRow<FArmorKitRow>(RowNames[0], "", false); RowBool != nullptr)
						{
							TArray<FArmorKitRow*> Rows{};
							TableToDrawFrom->GetAllRows({}, Rows);

							const int Index = Draw - (CumulativeNumberOfItems[j] - Rows.Num());

							Stock.Add(GameMode->GenerateArmorKitInfo(RowNames[Index], *Rows[Index], Level + ItemPool.ItemsLevelDelta));
						}
						if(const FWeaponKitRow* RowBool = TableToDrawFrom->FindRow<FWeaponKitRow>(RowNames[0], "", false); RowBool != nullptr)
						{
							TArray<FWeaponKitRow*> Rows{};
							TableToDrawFrom->GetAllRows({}, Rows);

							const int Index = Draw - (CumulativeNumberOfItems[j] - Rows.Num());

							Stock.Add(GameMode->GenerateWeaponKitInfo(RowNames[Index], *Rows[Index], Level + ItemPool.ItemsLevelDelta));
						}
						//----
						if (const FBeltRow* RowBool = TableToDrawFrom->FindRow<FBeltRow>(RowNames[0], "", false); RowBool != nullptr)
						{
							TArray<FBeltRow*> Rows{};
							TableToDrawFrom->GetAllRows({}, Rows);

							const int Index = Draw - (CumulativeNumberOfItems[j] - Rows.Num());

							Stock.Add(GameMode->GenerateBeltInfo(RowNames[Index], *Rows[Index], Level + ItemPool.ItemsLevelDelta));
						}

						if (const FGlovesRow* RowBool = TableToDrawFrom->FindRow<FGlovesRow>(RowNames[0], "", false); RowBool != nullptr)
						{
							TArray<FGlovesRow*> Rows{};
							TableToDrawFrom->GetAllRows({}, Rows);

							const int Index = Draw - (CumulativeNumberOfItems[j] - Rows.Num());

							Stock.Add(GameMode->GenerateGlovesInfo(RowNames[Index], *Rows[Index], Level + ItemPool.ItemsLevelDelta));
						}

						if (const FNecklaceRow* RowBool = TableToDrawFrom->FindRow<FNecklaceRow>(RowNames[0], "", false); RowBool != nullptr)
						{
							TArray<FNecklaceRow*> Rows{};
							TableToDrawFrom->GetAllRows({}, Rows);

							const int Index = Draw - (CumulativeNumberOfItems[j] - Rows.Num());

							Stock.Add(GameMode->GenerateNecklaceInfo(RowNames[Index], *Rows[Index], Level + ItemPool.ItemsLevelDelta));
						}

						if (const FEarringsRow* RowBool = TableToDrawFrom->FindRow<FEarringsRow>(RowNames[0], "", false); RowBool != nullptr)
						{
							TArray<FEarringsRow*> Rows{};
							TableToDrawFrom->GetAllRows({}, Rows);

							const int Index = Draw - (CumulativeNumberOfItems[j] - Rows.Num());

							Stock.Add(GameMode->GenerateEarringsInfo(RowNames[Index], *Rows[Index], Level + ItemPool.ItemsLevelDelta));
						}

						if (const FRingRow* RowBool = TableToDrawFrom->FindRow<FRingRow>(RowNames[0], "", false); RowBool != nullptr)
						{
							TArray<FRingRow*> Rows{};
							TableToDrawFrom->GetAllRows({}, Rows);

							const int Index = Draw - (CumulativeNumberOfItems[j] - Rows.Num());

							Stock.Add(GameMode->GenerateRingInfo(RowNames[Index], *Rows[Index], Level + ItemPool.ItemsLevelDelta));
						}
						//--
						break;
					}
				}
			}

		}
	}
}
