#include "EfhorisGameInstance.h"

#include "AbilitySystemGlobals.h"
#include "EfhorisPlayerController.h"
#include "OnlineSubsystem.h"
#include "ProjetHiver/BDD/DataBaseSubsystem.h"
#include "ProjetHiver/LobbyCore/LobbyPlayerState.h"
#include "ProjetHiver/LobbyCore/LobbyGameStateBase.h"
#include "ProjetHiver/LobbyCore/LobbyPlayerController.h"
#include "ProjetHiver/Save/GameSave.h"
#include "ProjetHiver/Save/SaveSubsystem.h"
#include "ProjetHiver/Widgets/Lobby/LobbyLevel.h"
#include "ProjetHiver/Widgets/FullscreenWidget.h"
#include "SocketSubsystem.h"

const FName SessionBaseName = FName("Dragon's curse session");

const FString LobbiesBaseName = FString("DragonsCurseLobby");

void UEfhorisGameInstance::Init() 
{
	Super::Init();	

	UAbilitySystemGlobals::Get().InitGlobalData();

	if (Subsystem = IOnlineSubsystem::Get(); ensure(Subsystem)) {
		if (SessionInterface = Subsystem->GetSessionInterface(); ensure(SessionInterface.IsValid())) {
			SessionInterface->OnSessionParticipantsChangeDelegates.AddUObject(this, &UEfhorisGameInstance::OnSessionParticipantsChange);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UEfhorisGameInstance::OnDestroySessionComplete);

			GetSubsystem<USaveSubsystem>()->InitSaverThread();

			if (IsDedicatedServerInstance()) {
				CreateServer();
			} else {
				SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UEfhorisGameInstance::OnFindSessionComplete);

				if (Friends = Subsystem->GetFriendsInterface(); Friends.IsValid())
					if (Identity = Subsystem->GetIdentityInterface(); Identity.IsValid()) {
					Login();
				}
			}
		}
	}
}

void UEfhorisGameInstance::Shutdown()
{
	GetSubsystem<USaveSubsystem>()->StopSaverThread();

	Super::Shutdown();
}

// Authentification et gestion des comptes

void UEfhorisGameInstance::Login()
{
	if (ensure(Identity.IsValid())) {
		FOnlineAccountCredentials Credentials;
		Credentials.Id = FString("localhost:8081");
		Credentials.Token = FString("EfhorisCredName");
		Credentials.Type = FString("developer");
		//Credentials.Type = FString("accountportal");

		Identity->OnLoginCompleteDelegates->AddUObject(this, &UEfhorisGameInstance::OnLoginComplete);
		Identity->Login(0, Credentials);
	}
}

void UEfhorisGameInstance::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	UE_LOG(LogTemp, Warning, TEXT("LoggedIn : %d with UserId : %s, Error : %s"), bWasSuccessful, *UserId.ToString(), *Error);

	if (bIsLoggedIn = bWasSuccessful; bIsLoggedIn)
	{
		Identity->ClearOnLoginCompleteDelegates(0, this);
		ConnectToHttp();
	}
	else {
		FOnlineAccountCredentials Credentials;
		Credentials.Type = FString("accountportal");

		Identity->Login(0, Credentials);
	}
}


void UEfhorisGameInstance::GetAllFriends()
{
	if (bIsLoggedIn)
		if (Friends.IsValid())
			Friends->ReadFriendsList(0, FString(""), FOnReadFriendsListComplete::CreateUObject(this, &UEfhorisGameInstance::OnReadFriendListComplete));
}

void UEfhorisGameInstance::OnReadFriendListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	UE_LOG(LogTemp, Warning, TEXT("Friend List Readed : %d, Error : %s"), bWasSuccessful, *ErrorStr);

	if (bWasSuccessful) {
		TArray<TSharedRef<FOnlineFriend>> FriendList;

		Friends->GetFriendsList(LocalUserNum, ListName, FriendList);

		for (TSharedRef<FOnlineFriend> Friend : FriendList) {
			UE_LOG(LogTemp, Warning, TEXT("Friend Real Name : %s, Display Name : %s"), *Friend->GetRealName(), *Friend->GetDisplayName())

				// TODO : faire quelque chose avec cette liste d'amis
		}
	}
}



// Partie Creation et Lancement de session

void UEfhorisGameInstance::CreateServer(bool bPrivate)
{
	if (bIsLoggedIn || IsDedicatedServerInstance())
		if (SessionInterface.IsValid()) {
			UE_LOG(LogTemp, Warning, TEXT("Create Server"));

			FOnlineSessionSettings SessionSettings;
			SessionSettings.bAllowJoinInProgress = true;
			SessionSettings.bAllowJoinViaPresence = true;
			SessionSettings.bIsDedicated = IsDedicatedServerInstance();
			SessionSettings.bIsLANMatch = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");
			SessionSettings.bShouldAdvertise = true;
			if (IsDedicatedServerInstance()) {
				SessionSettings.bUseLobbiesIfAvailable = false;
				SessionSettings.bUsesPresence = false;
			} else {
				SessionSettings.bUseLobbiesIfAvailable = true;
				SessionSettings.bUsesPresence = true;
			}
			if (bPrivate) SessionSettings.NumPrivateConnections = NbConnections;
			else SessionSettings.NumPublicConnections = NbConnections;

			SessionSettings.Set(SEARCH_KEYWORDS, LobbiesBaseName, EOnlineDataAdvertisementType::ViaOnlineService);
			if (IsDedicatedServerInstance()) {
				SessionSettings.Set(SETTING_MAPNAME, FString("CreateMainMenu"), EOnlineDataAdvertisementType::ViaOnlineService);
			}

			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UEfhorisGameInstance::OnCreateSessionComplete);
			SessionInterface->CreateSession(0, SessionBaseName, SessionSettings);
		}
}

void UEfhorisGameInstance::OnCreateSessionComplete(FName SessionName, bool Succeeded)
{
	UE_LOG(LogTemp, Warning, TEXT("Session Created : %d"), Succeeded)

	if (bHasCreatedTheSession = Succeeded; bHasCreatedTheSession) {
		SessionInterface->ClearOnCreateSessionCompleteDelegates(this);

		SessionNameClient = SessionName;

		SessionInterface->OnStartSessionCompleteDelegates.AddUObject(this, &UEfhorisGameInstance::OnStartSessionComplete);
		SessionInterface->StartSession(SessionNameClient);
	}
}


void UEfhorisGameInstance::ToggleReady()
{
	if (ALobbyPlayerController* PController = GetWorld()->GetFirstPlayerController<ALobbyPlayerController>(); IsValid(PController)) {
		UE_LOG(LogTemp, Verbose, TEXT("Toggle Ready"));
		PController->Server_ToggleReady();
	}
}

void UEfhorisGameInstance::SetReady(bool b)
{
	if (ALobbyPlayerController* PController = GetWorld()->GetFirstPlayerController<ALobbyPlayerController>(); IsValid(PController)) {
		if (ALobbyPlayerState* PlayerState = PController->GetPlayerState<ALobbyPlayerState>(); IsValid(PlayerState)) {
			UE_LOG(LogTemp, Verbose, TEXT("Set Ready : %d"), b);
			PlayerState->SetIsReady(b);
		}
	}
}

bool UEfhorisGameInstance::ArePlayersReady()
{
	UE_LOG(LogTemp, Log, TEXT("Are Players Ready"));
	if (ALobbyGameStateBase* GameState = GetWorld()->GetGameState<ALobbyGameStateBase>(); IsValid(GameState)) {
		bool R = true;
		for (auto& Player : GameState->PlayerArray) {
			if (ALobbyPlayerState* PlayerState = Cast<ALobbyPlayerState>(Player.Get()); IsValid(PlayerState))
				R &= PlayerState->IsReady();
			else
				return false;
		}
		return R;
	}
	return false;
}

void UEfhorisGameInstance::LaunchSession(const FString& GameName)
{
	bool bArePlayersReady = ArePlayersReady();
	if (bHasCreatedTheSession) {
		if (bArePlayersReady) {
			UE_LOG(LogTemp, Warning, TEXT("Launch Session"));

			if (ALobbyPlayerController* PController = GetWorld()->GetFirstPlayerController<ALobbyPlayerController>(); IsValid(PController)) {
				PController->Server_LaunchSession(GameName);
				PController->Server_EveryoneOpenLoadingScreen();
			}
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Every Players Are Not Ready!"));
		}
	}
}

void UEfhorisGameInstance::OnStartSessionComplete(FName SessionName, bool Succeeded)
{
	UE_LOG(LogTemp, Warning, TEXT("Session Started : %d"), Succeeded);
	
	if (bHasBeenStarted = Succeeded; bHasBeenStarted) {
		SessionInterface->ClearOnStartSessionCompleteDelegates(this);

		GetWorld()->ServerTravel("?listen", TRAVEL_Absolute);
	}
}

void UEfhorisGameInstance::DestroyServer()
{
	if (bIsLoggedIn)
		if (SessionInterface.IsValid()) {
			UE_LOG(LogTemp, Warning, TEXT("Destroy Server"));

			if (bHasBeenStarted) {
				SessionInterface->OnEndSessionCompleteDelegates.AddUObject(this, &UEfhorisGameInstance::OnEndSessionComplete);
				SessionInterface->EndSession(SessionNameClient);
			}

			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UEfhorisGameInstance::OnDestroySessionComplete);
			SessionInterface->DestroySession(SessionNameClient);
		}
	
}

void UEfhorisGameInstance::OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("Session Ended : %d"), bWasSuccessful);

	if (bHasBeenStarted = !bWasSuccessful; !bHasBeenStarted) {
		SessionInterface->ClearOnEndSessionCompleteDelegates(this);
	}
}

void UEfhorisGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("Session Destroyed : %d"), bWasSuccessful);

	if (bWasSuccessful) {
		bHasCreatedTheSession = bHasJoinedTheSession = false;

		ResetShouldLoadGame();

		USaveSubsystem* SaveSubsystem = GetSubsystem<USaveSubsystem>();
		SaveSubsystem->ResetSaves();

		if (APlayerController* PController = GetWorld()->GetFirstPlayerController()) {
			PController->ClientTravel("/Game/Efhoris/Multiplayer/CreateMainMenu?closed", ETravelType::TRAVEL_Absolute);
		}
	}
}

void UEfhorisGameInstance::OnSessionParticipantsChange(FName SessionName, const FUniqueNetId& UniqueId, bool bJoined)
{
	UE_LOG(LogTemp, Warning, TEXT("Session Participants Change, has joined : %d"), bJoined);
}



// Partie Recherche de session

void UEfhorisGameInstance::LookForServers()
{
	if (bIsLoggedIn)
		if (SessionInterface.IsValid())
			LookForDedicatedServers();
}

void UEfhorisGameInstance::LookForDedicatedServers()
{
	UE_LOG(LogTemp, Verbose, TEXT("Look For Dedicated Servers"));

	// Look for Dedicated Servers
	bHasLookedForListenServ = false;

	SessionSearchDedicated = MakeShareable(new FOnlineSessionSearch());
	SessionSearchDedicated->bIsLanQuery = true;
	SessionSearchDedicated->MaxSearchResults = 2000;
	SessionSearchDedicated->QuerySettings.Set(SEARCH_KEYWORDS, LobbiesBaseName, EOnlineComparisonOp::Equals);
	SessionSearchDedicated->QuerySettings.Set(SEARCH_DEDICATED_ONLY, true, EOnlineComparisonOp::Equals);

	SessionInterface->FindSessions(0, SessionSearchDedicated.ToSharedRef());
}

void UEfhorisGameInstance::LookForListenServers()
{
	UE_LOG(LogTemp, Verbose, TEXT("Look For Listen Servers"));

	// Look for Listen Servers
	bHasLookedForListenServ = true;

	SessionSearchListen = MakeShareable(new FOnlineSessionSearch());
	SessionSearchListen->bIsLanQuery = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");
	SessionSearchListen->QuerySettings.Set(SEARCH_KEYWORDS, LobbiesBaseName, EOnlineComparisonOp::Equals);
	SessionSearchListen->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	SessionSearchListen->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	SessionInterface->FindSessions(0, SessionSearchListen.ToSharedRef());
}

void UEfhorisGameInstance::OnFindSessionComplete(bool Succeeded)
{
	using namespace EOnlineAsyncTaskState;
	TSharedPtr<FOnlineSessionSearch>& SessionSearch = bHasLookedForListenServ ? SessionSearchListen : SessionSearchDedicated;

	EOnlineAsyncTaskState::Type SearchState = SessionSearch->SearchState;

	FString Msg;
	switch (SearchState) {
	case NotStarted:	Msg = "The task has not been started"; break;
	case InProgress:	Msg = "The task is currently being processed"; break;
	case Done:			Msg = "The task has completed successfully"; break;
	case Failed:		Msg = "The task failed to complete"; break;
	}

	UE_LOG(LogTemp, Verbose, TEXT("On Find Sessions : %s"), *Msg);

	if (Succeeded)
	{
		TArray<FOnlineSessionSearchResult> SearchResults = SessionSearch->SearchResults;
		UE_LOG(LogTemp, Verbose, TEXT("Found %d Sessions"), SearchResults.Num());

		for (auto& Session : SessionInfos)
			Session.bUpdated = false;

		for (int i = 0; i < SearchResults.Num(); ++i) {
			FOnlineSessionSearchResult& Res = SearchResults[i];
			FSessionInfo Infos;

			Infos.SessionName = Res.GetSessionIdStr();
			Infos.HostName = Res.Session.OwningUserName;
			Infos.MaxPlayerNumber = Res.Session.SessionSettings.NumPublicConnections +
				Res.Session.SessionSettings.NumPrivateConnections;
			Infos.PlayerNumber = Res.Session.SessionSettings.NumPublicConnections +
				Res.Session.SessionSettings.NumPrivateConnections -
				(Res.Session.NumOpenPrivateConnections + Res.Session.NumOpenPublicConnections);
			Infos.bIsPrivate = (Res.Session.SessionSettings.NumPrivateConnections == 0);
			Infos.bIsDedicated = !bHasLookedForListenServ;
			Infos.bUpdated = true;

			UE_LOG(LogTemp, VeryVerbose, TEXT("Session nb %d found = \"%s\""), i, *Res.GetSessionIdStr());

			if (int Index; SessionInfos.Find(Infos, Index))
				SessionInfos[Index] = Infos;
			else
				SessionInfos.Add(Infos);
		}

		// Retirer les sessions fermees
		for (int i = 0; i < SessionInfos.Num();) {
			FSessionInfo& Info = SessionInfos[i];

			if (((Info.bIsDedicated && !bHasLookedForListenServ) ||
				(!Info.bIsDedicated && bHasLookedForListenServ)) && !Info.bUpdated)
				SessionInfos.RemoveAt(i);
			else ++i;
		}
	}

	if (!bHasLookedForListenServ)
		LookForListenServers();
}


void UEfhorisGameInstance::JoinServer(FName SessionName,  FSessionInfo Session)
{
	if (bIsLoggedIn)
		if (SessionInterface.IsValid()) {
			UE_LOG(LogTemp, Log, TEXT("Join Server"));

			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UEfhorisGameInstance::OnJoinSessionComplete);

			TSharedPtr<FOnlineSessionSearch>& SessionSearch = Session.bIsDedicated ? SessionSearchDedicated : SessionSearchListen;

			for (auto& SessionInfo : SessionSearch->SearchResults) {
				if (SessionInfo.GetSessionIdStr() == Session.SessionName) {
					SessionInterface->JoinSession(0, SessionName, SessionInfo);
					if (ALobbyPlayerController* PController = GetWorld()->GetFirstPlayerController<ALobbyPlayerController>(); IsValid(PController))
					{
						PController->Client_OpenLoadingScreen();
					}
					return;
				}
			}

			UE_LOG(LogTemp, Error, TEXT("The session wasn't found"));
			SessionInfos.Remove(Session);
			
		}
}

void UEfhorisGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	using namespace EOnJoinSessionCompleteResult;
	FString Msg;
	switch (Result) {
	case Success: Msg = "The join worked as expected"; break;
	case SessionIsFull: Msg = "There are no open slots to join"; break;
	case SessionDoesNotExist: Msg = "The session couldn't be found on the service"; break;
	case CouldNotRetrieveAddress: Msg = "There was an error getting the session server's address"; break;
	case AlreadyInSession: Msg = "The user attempting to join is already a member of the session"; break;
	case UnknownError: Msg = " An error not covered above occurred"; break;
	}

	UE_LOG(LogTemp, Log, TEXT("Join Server : %s"), *Msg);

	if (Result == Success) {
		if (ALobbyPlayerController* PController = GetWorld()->GetFirstPlayerController<ALobbyPlayerController>(); IsValid(PController))
		{
			FString JoinAddress = "";
			SessionInterface->GetResolvedConnectString(SessionName, JoinAddress);
			if (JoinAddress != "")
			{
				bHasJoinedTheSession = true;
				SessionNameClient = SessionName;

				PController->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}

// Partie Sauvegarde

void UEfhorisGameInstance::GetAllGameSave() const
{
	UDataBaseSubsystem* DataBaseSubSystem = GetSubsystem<UDataBaseSubsystem>();

	if (DataBaseSubSystem->IsLoggedIn()) {
		UE_LOG(LogTemp, Warning, TEXT("Looking for saves on the data base"));

		DataBaseSubSystem->GetAllSaves();
	} else {
		UE_LOG(LogTemp, Warning, TEXT("Looking for saves on local folder"));

		USaveSubsystem* SaveSubsystem = GetSubsystem<USaveSubsystem>();

		SetAllGameSaveFromDataBase(SaveSubsystem->GetAllSaves());
	}
}

void UEfhorisGameInstance::LoadGame(const FGameSave& GameSave_)
{
	if (ALobbyPlayerController* PController = GetWorld()->GetFirstPlayerController<ALobbyPlayerController>(); IsValid(PController)) {
		GameSave = GameSave_;

		PController->Client_OpenLoadingScreen();
		PController->Server_SetGameLoaded(GameSave);

		const USaveSubsystem* SaveSubsystem = GetSubsystem<USaveSubsystem>();
		SaveSubsystem->LoadGame(GameSave.GameId.ToString());
		
		if (ALobbyLevel* LobbyLevelScript = Cast<ALobbyLevel>(PController->GetLevel()->GetLevelScriptActor()); IsValid(LobbyLevelScript)) {
			LobbyLevelScript->GetMainWidget()->BackToLobby();
		}
	}
}

void UEfhorisGameInstance::UnLoadGame()
{
	GameSave = FGameSave();

	if (ALobbyPlayerController* PController = GetWorld()->GetFirstPlayerController<ALobbyPlayerController>(); IsValid(PController)) {
		PController->Server_ResetGameLoaded();
	}
}

void UEfhorisGameInstance::DeleteSave(const FGameSave& GameSave_)
{
	USaveSubsystem* SaveSubsystem = GetSubsystem<USaveSubsystem>();

	if (GameSave_ == GameSave)
		UnLoadGame();

	SaveSubsystem->DeleteSave(GameSave_);
}

void UEfhorisGameInstance::ConnectToHttp() const noexcept
{
	GetSubsystem<UDataBaseSubsystem>()->Init(GetPlayerId());
}

const UCompositeDataTable* UEfhorisGameInstance::GetArmorKitsDataTable() const
{
	return ArmorKitsDataTable;
}

const UCompositeDataTable* UEfhorisGameInstance::GetTrashItemsDataTable() const
{
	return TrashItemsDataTable;
}

const UCompositeDataTable* UEfhorisGameInstance::GetWeaponKitsDataTable() const
{
	return WeaponKitsDataTable;
}

const UCompositeDataTable* UEfhorisGameInstance::GetGlovesDataTable() const
{
	return GlovesDataTable;
}

const UCompositeDataTable* UEfhorisGameInstance::GetBeltsDataTable() const
{
	return BeltsDataTable;
}

const UCompositeDataTable* UEfhorisGameInstance::GetRingsDataTable() const
{
	return RingsDataTable;
}

const UCompositeDataTable* UEfhorisGameInstance::GetNecklacesDataTable() const
{
	return NecklacesDataTable;
}

const UCompositeDataTable* UEfhorisGameInstance::GetEarringsDataTable() const
{
	return EarringsDataTable;
}

const UCompositeDataTable* UEfhorisGameInstance::GetMerchantsDataTable() const
{
	return MerchantsDataTable;
}

const UCompositeDataTable* UEfhorisGameInstance::GetLootsDataTable() const
{
	return LootsDataTable;
}

void UEfhorisGameInstance::RefreshLobby()
{
	if (!IsDedicatedServerInstance()) {
		if(bIsLoggedIn && (bHasJoinedTheSession || bHasCreatedTheSession))
		if (const UWorld* World = GetWorld(); IsValid(World)) {
			if (const ALobbyGameStateBase* GameState = World->GetGameState<ALobbyGameStateBase>(); IsValid(GameState)) {
				UE_LOG(LogTemp, Log, TEXT("RefreshLobby, %d players"), GameState->PlayerArray.Num());
					
				TArray<FLobbyInfo> Infos;

				ALobbyPlayerState* MyPlayerState = World->GetFirstPlayerController()->GetPlayerState<ALobbyPlayerState>();

				for (auto& Player : GameState->PlayerArray) {
					if (ALobbyPlayerState* PlayerState = Cast<ALobbyPlayerState>(Player.Get()); IsValid(PlayerState)) {
						if (PlayerState != MyPlayerState) {
							FLobbyInfo Info;

							Info.NickName = PlayerState->GetNickName();
							Info.bIsReady = PlayerState->IsReady();

							UE_LOG(LogTemp, Log, TEXT("\tPlayer name = %s"), *Info.NickName);

							Infos.Add(Info);
						}
					}
				}

				if (ALobbyLevel* LobbyLevelScript = Cast<ALobbyLevel>(World->GetFirstPlayerController()->GetLevel()->GetLevelScriptActor()); IsValid(LobbyLevelScript)) {		
					if (bShowLaunchButton) {
						bShowLaunchButton = false;
						LobbyLevelScript->GetMainWidget()->ShowLaunchButton();
					}
					
					LobbyLevelScript->GetMainWidget()->RefreshLobby(Infos);
				}
			}
		}
	}

}

void UEfhorisGameInstance::SetAllGameSaveFromDataBase(TArray<FGameSave> GameSaves) const
{
	if (const UWorld* World = GetWorld(); IsValid(World))
		if (ALobbyLevel* LobbyLevelScript = Cast<ALobbyLevel>(World->GetFirstPlayerController()->GetLevel()->GetLevelScriptActor()); IsValid(LobbyLevelScript))
			LobbyLevelScript->GetMainWidget()->RefreshGameSaves(GameSaves);
}