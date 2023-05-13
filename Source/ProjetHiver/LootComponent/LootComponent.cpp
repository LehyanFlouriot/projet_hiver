#include "LootComponent.h"

#include "Components/BoxComponent.h"
#include "Engine/CompositeDataTable.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameInstance.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/Inventory/LootChest.h"
#include "ProjetHiver/Items/HealSeed.h"
#include "ProjetHiver/Items/ItemType.h"
#include "ProjetHiver/Items/PickableItem.h"
#include "ProjetHiver/MapGeneration/LevelGenerator.h"

void ULootComponent::BeginPlay()
{
	Super::BeginPlay();


	if (const AEfhorisGameState* EfhorisGameState = GetWorld()->GetGameState<AEfhorisGameState>())
		if (const ALevelGenerator* LevelGenerator = EfhorisGameState->GetLevelGenerator())
			Level = LevelGenerator->CurrentLevel + LootLevelOffset;

	for (const FPullInfo& PullInfo : PullInfos)
	{
		LootTableRows.Add(*CastChecked<UEfhorisGameInstance>(GetWorld()->GetGameInstance())->GetLootsDataTable()->FindRow<FLootTableRow>(PullInfo.RowName, {}));
		TotalWeights.Add(CalculateTotalWeight(LootTableRows.Last().WeightedItems));
	}
}

AActor* ULootComponent::GenerateItem(const EItemType Type, const FName& Name) const
{
	if (GetOwner()->HasAuthority())
	{
		const int LootLevel = FMath::RandRange(0, 99) >= 90 ? Level+1 : Level;
		if (AEfhorisGameMode* GameMode = GetWorld()->GetAuthGameMode<AEfhorisGameMode>(); IsValid(GameMode))
		{
			switch (Type)
			{
			case EItemType::Armor:
				return GameMode->SpawnArmorKit(Name, LootOrigin, LootLevel, true);
			case EItemType::Weapon:
				return GameMode->SpawnWeaponKit(Name, LootOrigin, LootLevel, true);
			case EItemType::Trash:
				return GameMode->SpawnTrashItem(Name, LootOrigin, LootLevel);
			case EItemType::HealSeed:
				return GameMode->SpawnHealSeed(LootOrigin);
			case EItemType::Belt:
				return GameMode->SpawnBelt(Name, LootOrigin, LootLevel, true);
			case EItemType::Gloves:
				return GameMode->SpawnGloves(Name, LootOrigin, LootLevel, true);
			case EItemType::Earrings:
				return GameMode->SpawnEarrings(Name, LootOrigin, LootLevel, true);
			case EItemType::Necklace:
				return GameMode->SpawnNecklace(Name, LootOrigin, LootLevel, true);
			case EItemType::Ring:
				return GameMode->SpawnRing(Name, LootOrigin, LootLevel, true);
			default:
				break;
			}
		}
	}
	return nullptr;
}

void ULootComponent::SetSpawnValues()
{
	const FTransform ActorTransform = GetOwner()->GetActorTransform();
	LootOrigin = ActorTransform;
	LootOrigin.AddToTranslation(LootSpawnOffset);
	VelocityDirection = VelocityDirection.RotateAngleAxis(ActorTransform.GetRotation().Rotator().Yaw, FVector(0, 0, 1));
}

void ULootComponent::GenerateLoot()
{
	SetSpawnValues();

	for (int i = 0; i < PullInfos.Num(); ++i)
	{
		const FPullInfo& PullInfo = PullInfos[i];
		int PullsRemaining = PullInfo.PullNumber;

		while (PullsRemaining > 0)
		{
			const float RandomAmount = FMath::RandRange(0.0f, 1.0f);

			if (RandomAmount <= PullInfo.Probability)
			{
				float RandomWeight = FMath::RandRange(0.0f, TotalWeights[i]);
				for (const auto& WeightedItem : LootTableRows[i].WeightedItems)
				{
					RandomWeight -= WeightedItem.Weight;
					if (RandomWeight < 0)
					{
						AActor* Item = GenerateItem(WeightedItem.Type, WeightedItem.Name);
						FVector Direction = VelocityDirection;
						Direction = Direction.RotateAngleAxis(FMath::RandRange(-LootConeAngleDegree, LootConeAngleDegree), GetOwner()->GetActorTransform().GetRotation().GetUpVector());
						Direction = Direction.RotateAngleAxis(FMath::RandRange(-LootConeAngleDegree, LootConeAngleDegree), GetOwner()->GetActorTransform().GetRotation().GetRightVector());

						if (const APickableItem* PickableItem = Cast<APickableItem>(Item)) {
							AddItemImpulse(PickableItem, LootSpeed * Direction);
						}
						else if (AHealSeed* HealSeed = Cast<AHealSeed>(Item))
						{
							if (const ALootChest* Chest = Cast<ALootChest>(GetOwner()); IsValid(Chest)) {
								if (const auto HealSeedLocation = Chest->GetSeedLocation(); IsValid(HealSeedLocation)) {
									HealSeed->SetActorLocation(HealSeedLocation->GetComponentLocation());
								}
							}
						}

						break;
					}
				}
			}

			PullsRemaining--;
		}
	}
}

float ULootComponent::CalculateTotalWeight(const TArray<FWeightedItem>& WeightedItems)
{
	float Sum = 0.0f;

	for (const auto& WeightedItem : WeightedItems)
		Sum += WeightedItem.Weight;

	return Sum;
}

void ULootComponent::AddItemImpulse(const APickableItem* Item, const FVector& Impulse) {
	if (IsValid(Item)) {
		if (UBoxComponent* Box = Item->GetCollision(); IsValid(Box)) {
			Box->AddImpulse(Impulse, NAME_None, true);
		}
	}
}
