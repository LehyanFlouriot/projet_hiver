#include "LootChest.h"

#include "Net/UnrealNetwork.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/Contracts/AccumulationContractType.h"
#include "ProjetHiver/Contracts/ContractStatus.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/LootComponent/LootComponent.h"

void ALootChest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALootChest, bIsInteractible);
	DOREPLIFETIME(ALootChest, bIsOpened);
}

ALootChest::ALootChest(const FObjectInitializer& ObjectInitializer)
{
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent")));
	GetRootComponent()->SetMobility(EComponentMobility::Static);

	LootComponent = CreateDefaultSubobject<ULootComponent>(TEXT("LootComponent"));
	LootComponent->SetupAttachment(RootComponent);

	HealSeedLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Heal Seed Location"));
	HealSeedLocation->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ALootChest::BeginPlay()
{
	Super::BeginPlay();
}

bool ALootChest::IsInteractible(const APlayerCharacter* Player) const {
	return Player->IsAliveTag() && bIsInteractible;
}

void ALootChest::StartInteract(APlayerCharacter* Player)
{
	bIsInteractible = false;
	LootComponent->GenerateLoot();
	bIsOpened = true;

	if (const AEfhorisGameState* GameState = GetWorld()->GetGameState<AEfhorisGameState>(); IsValid(GameState))
		if (AAccumulationContract* Contract = GameState->GetAccumulationContract(); IsValid(Contract))
			if (Contract->GetType() == EAccumulationContractType::Chest && Contract->GetStatus() == EContractStatus::OnGoing)
				Contract->IncreaseCount();
}

void ALootChest::StopInteract(APlayerCharacter* Player) {}

const FText ALootChest::GetInteractionText() const
{
	return InteractionText;
}

USceneComponent* ALootChest::GetSeedLocation() const {
	return HealSeedLocation;
}

void ALootChest::Open()
{
	bIsInteractible = false;
	bIsOpened = true;
}
