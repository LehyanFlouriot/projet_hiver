#include "Kevin.h"

#include "AIController.h"
#include "ContractStatus.h"
#include "ExplorationContractType.h"
#include "Net/UnrealNetwork.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerState.h"

AKevin::AKevin(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bReplicates = true;
	ACharacter::SetReplicateMovement(true);
}

bool AKevin::IsInteractible(const APlayerCharacter* Player) const
{
	bool bIsInteractible = false;

	if (const AEfhorisGameState* GameState = GetWorld()->GetGameState<AEfhorisGameState>(); IsValid(GameState))
		if (AExplorationContract* Contract = GameState->GetExplorationContract(); IsValid(Contract))
			if ((Contract->GetStatus() == EContractStatus::OnGoing || Contract->GetStatus() == EContractStatus::Completable) && Contract->GetType() == EExplorationContractType::FetchNPC)
				bIsInteractible = true;

	return Player->IsAliveTag() && bIsInteractible && (!IsValid(Target) || (IsValid(Target) && Target != Player));
}

void AKevin::StartInteract(APlayerCharacter* Player)
{
	if (const AEfhorisGameState* GameState = GetWorld()->GetGameState<AEfhorisGameState>(); IsValid(GameState))
	{
		if (AExplorationContract* Contract = GameState->GetExplorationContract(); IsValid(Contract))
		{
			if (!IsValid(Target))
			{
				SetTarget(Player);
				Contract->SetStatus(EContractStatus::Completable);
			}
			else
			{
				GetController()->StopMovement();
				SetTarget(Player);
			}
		}
	}
}

void AKevin::StopInteract(APlayerCharacter* Player) {}

const FText AKevin::GetInteractionText() const
{
	if (IsValid(Target))
		return InteractionTextChangeOwner;
	return InteractionText;
}

void AKevin::ReachGoal()
{
	if (const AEfhorisGameState* GameState = GetWorld()->GetGameState<AEfhorisGameState>(); IsValid(GameState))
		if (AExplorationContract* Contract = GameState->GetExplorationContract(); IsValid(Contract))
			if (Contract->GetStatus() == EContractStatus::Completable && Contract->GetType() == EExplorationContractType::FetchNPC)
			{
				SetTarget(nullptr);
				Contract->SetStatus(EContractStatus::Succeeded);
			}
}

void AKevin::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AKevin, Target);
}

void AKevin::SetTarget(APlayerCharacter* Target_) noexcept
{
	Target = Target_;
	if (HasTarget()) {
		if (AEfhorisPlayerState* PlayerState_ = Target->GetPlayerState<AEfhorisPlayerState>(); IsValid(PlayerState_))
			SetTargetId(PlayerState_->GetSteamUserId());
	} else SetTargetId({});
}

bool AKevin::HasTarget() const noexcept
{
	return IsValid(Target);
}
