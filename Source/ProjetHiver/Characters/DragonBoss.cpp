#include "DragonBoss.h"

#include "Mercenary.h"
#include "PlayerCharacter.h"
#include "AttributeSets/HealthAttributeSet.h"
#include "Blueprint/UserWidget.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/AI/AIManager.h"
#include "ProjetHiver/BDD/UserScoreboardStatistic.h"
#include "ProjetHiver/EfhorisCore/EfhorisHUD.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerState.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"
#include "ProjetHiver/GAS/EfhorisAbilitySystemComponent.h"
#include "ProjetHiver/MapGeneration/LevelGenerator.h"
#include "ProjetHiver/Widgets/GameView.h"
#include "ProjetHiver/Widgets/HealthBoss.h"

void ADragonBoss::ClearUnitReferences() {
	HideHealthBar();
	if (const UWorld* World = GetWorld(); IsValid(World)) {
		if (const AEfhorisGameState* GameState = World->GetGameState<AEfhorisGameState>(); IsValid(GameState)) {
			if (AAIManager* Manager = GameState->GetAIManager(); IsValid(Manager)) {
				Manager->RemoveUnit(this);
				Manager->ClearMercenariesThreat(this);
			}
		}
	}
}

void ADragonBoss::K2_DestroyActor() {
	ClearUnitReferences();
	Super::K2_DestroyActor();
}

ADragonBoss::ADragonBoss(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
}

void ADragonBoss::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADragonBoss, RotationSpeed);
	DOREPLIFETIME(ADragonBoss, bIsFlying);
	DOREPLIFETIME(ADragonBoss, Phase);
	DOREPLIFETIME(ADragonBoss, bIsInCombat);
}

void ADragonBoss::BeginPlay()
{
	Super::BeginPlay();

	InitializeAbilities();
	InitializeAttributes();

	if (!HasAuthority())
	{
		if(const auto World = GetWorld(); IsValid(World))
		{
			if(const auto PC = World->GetFirstPlayerController(); IsValid(PC))
			{
				if(const auto HUD = PC->GetHUD<AEfhorisHUD>(); IsValid(HUD))
				{
					if(const auto GameView = HUD->GetGameView(); IsValid(GameView))
					{
						HealthBar = GameView->GetBossHealthBar();
					}
				}
			}
		}
	}

	InitialMaxHealth = GetMaxHealth();
	HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(HealthAttributes->GetHealthAttribute()).AddUObject(this, &ADragonBoss::HealthChanged);

	//AIManager
	if (AAIManager* Manager = GetWorld()->GetGameState<AEfhorisGameState>()->GetAIManager(); IsValid(Manager)) {
		Manager->AddUnit(this);
	}

	InitialLocation = GetActorLocation();
	InitialLocation.Z = 0;

	Phase = 1;

	bIsFlying = false;
	bIsInCombat = false;

	Landed({});
}

void ADragonBoss::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority()) {
		RotationSpeed = GetCapsuleComponent()->GetPhysicsAngularVelocityInDegrees().Z;
		SetActorRotation(FRotator{ 0, GetActorRotation().Yaw,  0});
	}
}

void ADragonBoss::Die()
{
	Super::Die();
	Multicast_SetCollision(ECollisionEnabled::NoCollision);
	ClearUnitReferences();
	GetController()->StopMovement();
	APawn::DetachFromControllerPendingDestroy();
	UE_LOG(LogTemp, Warning, TEXT("Dragon boss killed ! GG"));
	for (const auto& TempPlayerState : GetWorld()->GetGameState()->PlayerArray)
		if (AEfhorisPlayerState* EfhorisPlayerState = Cast<AEfhorisPlayerState>(TempPlayerState); IsValid(EfhorisPlayerState))
			EfhorisPlayerState->IncreaseStatistic(EUserScoreboardStatistic::BossesKilled, 1);

	if (AEfhorisGameState* EfhorisGameState = GetWorld()->GetGameState<AEfhorisGameState>(); IsValid(EfhorisGameState))
		EfhorisGameState->SetMoney(EfhorisGameState->GetMoney() + 10000 * EfhorisGameState->GetLevelGenerator()->CurrentLevel);

	ServerWinGameRequest();
}

void ADragonBoss::ServerWinGameRequest_Implementation()
{
	if (HasAuthority())
	{
		Cast<AEfhorisGameMode>(GetWorld()->GetAuthGameMode())->WinGame();
	}
}

void ADragonBoss::HealthChanged(const FOnAttributeChangeData& Data)
{
	const float Health = Data.NewValue;
	UpdateUIHealthBar();
}

void ADragonBoss::HideHealthBar_Implementation() {
	if (IsValid(HealthBar))
		HealthBar->EndBoss();
}

void ADragonBoss::ShowHealthBar_Implementation() {
	if (IsValid(HealthBar))
		HealthBar->StartBoss(this);
}

void ADragonBoss::IncreasePhase()
{
	++Phase;
	if (AEfhorisGameMode* GameMode = GetWorld()->GetAuthGameMode<AEfhorisGameMode>(); IsValid(GameMode))
	{
		if(Phase == 2)
			GameMode->SetMusicState(FName("SG_BossFight"), FName("Phase_02"));
		if (Phase == 3)
			GameMode->SetMusicState(FName("SG_BossFight"), FName("Phase_03"));
	}
}

void ADragonBoss::ScaleBoss(const int NbPlayers) const noexcept
{
	SetMaxHealth(InitialMaxHealth * NbPlayers);
}

void ADragonBoss::UpdateUIHealthBar_Implementation() {
	if (!HasAuthority() && HealthBar)
	{
		HealthBar->UpdateBoss();
	}
}
