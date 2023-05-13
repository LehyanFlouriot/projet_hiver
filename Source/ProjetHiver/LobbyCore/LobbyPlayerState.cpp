#include "LobbyPlayerState.h"

#include "Net/UnrealNetwork.h"
#include "ProjetHiver/BDD/DataBaseSubsystem.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameInstance.h"
#include "ProjetHiver/LobbyCore/LobbyGameStateBase.h"
#include "ProjetHiver/LobbyCore/LobbyPlayerController.h"

void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPlayerState, bIsReadyToLaunch);
	DOREPLIFETIME(ALobbyPlayerState, SteamNickName);
}

void ALobbyPlayerState::BeginPlay()
{
	Super::BeginPlay();

	Client_GetInformations();
}

void ALobbyPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	using namespace EEndPlayReason;
	if((EndPlayReason == Type::RemovedFromWorld)
		|| (EndPlayReason == Type::Destroyed)
		|| (EndPlayReason == Type::Quit))
		Multicast_RefreshLobby();
}

void ALobbyPlayerState::Client_GetInformations_Implementation()
{
	if (const UEfhorisGameInstance* GameInstance = Cast<UEfhorisGameInstance>(GetGameInstance()); IsValid(GameInstance)) {		
		Server_SetInformations(GameInstance->GetNickName(), GameInstance->GetPlayerId());
	}
}

void ALobbyPlayerState::Server_SetInformations_Implementation(const FString& NickName_, const FString& UserId_)
{
	SetNickName(NickName_);

	if (const UWorld* World = GetWorld(); IsValid(World))
		if (ALobbyGameStateBase* GameState = World->GetGameState<ALobbyGameStateBase>(); IsValid(GameState))
			if (GameState->IsLoadedGame())
				if (ALobbyPlayerController* PlayerController = Cast<ALobbyPlayerController>(GetPlayerController()); IsValid(PlayerController))
					PlayerController->Client_SetGameLoaded(GameState->GetGameName());
}

void ALobbyPlayerState::PlayerResfreshLobby() const
{
	if (UEfhorisGameInstance* GameInstance = Cast<UEfhorisGameInstance>(GetGameInstance()); IsValid(GameInstance)) {
		GameInstance->RefreshLobby();
	}
}

void ALobbyPlayerState::OnRep_RefreshLobby() const
{
	PlayerResfreshLobby();
}

void ALobbyPlayerState::Multicast_RefreshLobby_Implementation() const {
	PlayerResfreshLobby();
}
