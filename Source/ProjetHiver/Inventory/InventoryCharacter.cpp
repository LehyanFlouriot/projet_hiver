#include "InventoryCharacter.h"

#include "AIController.h"
#include "Net/UnrealNetwork.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerState.h"
#include "ProjetHiver/MapGeneration/LevelGenerator.h"

AInventoryCharacter::AInventoryCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bReplicates = true;
	bIsOpened = false;
	ACharacter::SetReplicateMovement(true);
}

bool AInventoryCharacter::IsInteractible(const APlayerCharacter* Player) const {
	return Player->IsAliveTag();
}

void AInventoryCharacter::StartInteract(APlayerCharacter* Player)
{
	if(HasAuthority())
	{
		// Afficher l'inventaire
		if(AEfhorisPlayerController* PlayerController = CastChecked<AEfhorisPlayerController>(Player->GetController());ensure(IsValid(PlayerController)))
		{
			PlayerController->Client_OpenInventoryPanel();
		}
		NbPlayerInteracting++;
		bIsOpened = true;
		GetController()->StopMovement();
	}
}

void AInventoryCharacter::StopInteract(APlayerCharacter* Player)
{
	if (HasAuthority())
	{
		// Cacher l'inventaire
		if (AEfhorisPlayerController* PlayerController = CastChecked<AEfhorisPlayerController>(Player->GetController()); ensure(IsValid(PlayerController)))
		{
			PlayerController->Client_CloseInventoryPanel();
		}
		NbPlayerInteracting = FMath::Max(NbPlayerInteracting-1,0);
		if(NbPlayerInteracting == 0)
			bIsOpened = false;
	}
}

const FText AInventoryCharacter::GetInteractionText() const
{
	return InteractionText;
}

void AInventoryCharacter::ResetRebuild() {
	NbPlayerInteracting = 0;
	bIsOpened = false;
	GetController()->StopMovement();
	RemoveTarget();
}

void AInventoryCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AInventoryCharacter, Target);
	DOREPLIFETIME(AInventoryCharacter, bIsOpened);
	DOREPLIFETIME(AInventoryCharacter, NbPlayerInteracting);
}

float AInventoryCharacter::DistanceToTarget() const
{
	if (!IsValid(Target)) return 0;
	return (Target->GetActorLocation() - GetActorLocation()).Size();
}

void AInventoryCharacter::TpChestNearTarget() {
	const UWorld* World = GetWorld();
	if (!IsValid(World) || !IsValid(Target)) return;
	const AEfhorisGameState* EGameState = World->GetGameState<AEfhorisGameState>();
	if (!IsValid(EGameState)) return;
	ALevelGenerator* LevelGenerator = EGameState->GetLevelGenerator();
	if (!IsValid(LevelGenerator)) return;
	
	const FVector2D ChestTile = FVector2D(FMath::RoundHalfToZero(GetActorLocation().X / LevelGenerator->TileSize.X), FMath::RoundHalfToZero(GetActorLocation().Y / LevelGenerator->TileSize.Y));
	const FVector2D TargetTile = FVector2D(FMath::RoundHalfToZero(Target->GetActorLocation().X / LevelGenerator->TileSize.X), FMath::RoundHalfToZero(Target->GetActorLocation().Y / LevelGenerator->TileSize.Y));

	if ( ChestTile == TargetTile || LevelGenerator->GetNeighborsPos(ChestTile).Contains(TargetTile)) return;

	const TArray<EConnection> NeighborsTarget = LevelGenerator->GetConnections(TargetTile);
	if (NeighborsTarget.IsEmpty()) return;

	const EConnection Connection = NeighborsTarget[FMath::RandRange(0,NeighborsTarget.Num()-1)];
	TeleportTo(LevelGenerator->GetConnectionWorldLocation(TargetTile, Connection) + FVector(0, 0, 100),FRotator());
}

void AInventoryCharacter::SetTarget(APlayerCharacter* Target_) {
	if (HasTarget() && Target_ == Target) return;
	if (AController* ChestController = GetController(); IsValid(ChestController))
		ChestController->StopMovement();
	if (HasTarget())
		RemoveTarget();
	if (!bIsOpened) {
		Target = Target_;
		if (IsValid(Target))
			if (const AEfhorisPlayerController* PlayerController = Target->GetController<AEfhorisPlayerController>(); IsValid(PlayerController))
				PlayerController->Client_ShowChestMoving();
		//Rubberband
		TpChestNearTarget();
	}
	if (HasTarget()) {
		if (AEfhorisPlayerState* PlayerState_ = Target->GetPlayerState<AEfhorisPlayerState>(); IsValid(PlayerState_))
			SetTargetId(PlayerState_->GetSteamUserId());
	} else SetTargetId({});
}

void AInventoryCharacter::RemoveTarget()
{
	if (IsValid(Target))
		if (const AEfhorisPlayerController* PlayerController = Target->GetController<AEfhorisPlayerController>(); IsValid(PlayerController))
			PlayerController->Client_HideChestMoving();
	Target = nullptr;
	SetTargetId({});
}
