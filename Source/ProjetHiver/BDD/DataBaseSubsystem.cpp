#include "DataBaseSubsystem.h"

#include "ProjetHiver/BDD/UserScoreboard.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameInstance.h"
#include "ProjetHiver/LobbyCore/LobbyPlayerController.h"
#include "ProjetHiver/Save/FileHelperSubsystem.h"
#include "ProjetHiver/Save/SaveSubsystem.h"

void UDataBaseSubsystem::Init(const FString& SteamId_)
{
	if (const UEfhorisGameInstance* GameInstance = Cast<UEfhorisGameInstance>(GetGameInstance()); IsValid(GameInstance)) {
		FString ServerIp = GameInstance->GetServerIP();

		ServerURL = "http://" + ServerIp + "/";// +":" + ServerPort + "/";
		SteamId = SteamId_;
		SteamIdEncoded = FGenericPlatformHttp::UrlEncode(SteamId);

		Http = &FHttpModule::Get();

		FileSubsystem = GetGameInstance()->GetSubsystem<UFileHelperSubsystem>();

		// Recuperer les donnees enregistrees.
		ReadDataBaseInfos();

		if (Infos)
			// Les donnees lues sont completes,
			// on verifie si l'utilisateur existe toujours.
			VerifyUserState();

		else
			// Les donnees lues sont incompletes, on se conecte.
			Login();
	}
}



void UDataBaseSubsystem::StoreSave(const FDataBaseSave& Save)
{
	if (IsLoggedIn()) {
		DataSave = Save;

		StoreGameSave(Save.GameId, Save.GameSave);
	}
	else {
		UE_LOG(LogTemp, Verbose, TEXT("Not logged in to the http server"))
	}
}

void UDataBaseSubsystem::LoadDirectorSave(const FString& GameId)
{
	if (IsLoggedIn()) {
		UE_LOG(LogTemp, Log, TEXT("Load Director Save"));

		TSharedRef Request = Http->CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &UDataBaseSubsystem::OnResponseLoadDirectorSave);
		FString GameIdEncoded = FGenericPlatformHttp::UrlEncode(GameId);
		Request->SetURL(ServerURL + "api/users/" + SteamIdEncoded + "/saves/" + GameIdEncoded + "/loadAI");
		Request->SetVerb("GET");
		FString AuthHeader = FString::Printf(TEXT("Bearer %s"), *Infos.SessionToken);
		Request->SetHeader("Authorization", AuthHeader);
		Request->ProcessRequest();
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Not logged in to the http server"))
	}
}

void UDataBaseSubsystem::LoadPlayerSaves(const FString& GameId)
{
	if (IsLoggedIn()) {
		UE_LOG(LogTemp, Log, TEXT("Load Player Saves"));

		TSharedRef Request = Http->CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &UDataBaseSubsystem::OnResponseLoadPlayerSaves);
		FString GameIdEncoded = FGenericPlatformHttp::UrlEncode(GameId);
		Request->SetURL(ServerURL + "api/users/" + SteamIdEncoded + "/saves/" + GameIdEncoded + "/loadPlayers");
		Request->SetVerb("GET");
		FString AuthHeader = FString::Printf(TEXT("Bearer %s"), *Infos.SessionToken);
		Request->SetHeader("Authorization", AuthHeader);
		Request->ProcessRequest();
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Not logged in to the http server"))
	}
}

void UDataBaseSubsystem::DeleteSave(const FString& GameId)
{
	if (IsLoggedIn()) {
		UE_LOG(LogTemp, Log, TEXT("Delete Save"));

		TSharedRef Request = Http->CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &UDataBaseSubsystem::OnResponseDeleteSave);
		FString GameIdEncoded = FGenericPlatformHttp::UrlEncode(GameId);
		Request->SetURL(ServerURL + "api/users/" + SteamIdEncoded + "/saves/" + GameIdEncoded + "/delete");
		Request->SetVerb("DELETE");
		FString AuthHeader = FString::Printf(TEXT("Bearer %s"), *Infos.SessionToken);
		Request->SetHeader("Authorization", AuthHeader);
		Request->ProcessRequest();
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Not logged in to the http server"))
	}
}

void UDataBaseSubsystem::GetAllSaves()
{
	if (IsLoggedIn()) {
		UE_LOG(LogTemp, Log, TEXT("Get All Saves"));

		TSharedRef Request = Http->CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &UDataBaseSubsystem::OnResponseGetAllSaves);
		Request->SetURL(ServerURL + "api/users/" + SteamIdEncoded + "/saves");
		Request->SetVerb("GET");
		FString AuthHeader = FString::Printf(TEXT("Bearer %s"), *Infos.SessionToken);
		Request->SetHeader("Authorization", AuthHeader);
		Request->ProcessRequest();
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Not logged in to the http server"))
	}
}



void UDataBaseSubsystem::GetPlayerScoreboard(const FString& PlayerId)
{
	if (IsLoggedIn()) {
		UE_LOG(LogTemp, Log, TEXT("Get Player Scoreboard"));

		TSharedRef Request = Http->CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &UDataBaseSubsystem::OnResponseGetPlayerScoreboard);
		Request->SetURL(ServerURL + "api/users/" + SteamIdEncoded + "/scoreboard");
		Request->SetVerb("GET");
		const FString AuthHeader = FString::Printf(TEXT("Bearer %s"), *Infos.SessionToken);
		Request->SetHeader("Authorization", AuthHeader);
		Request->ProcessRequest();
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Not logged in to the http server"))
	}
}

void UDataBaseSubsystem::IncreaseStatistics(const FString& PlayerId, const FUserScoreboard& Scoreboard)
{
	if (IsLoggedIn()) {
		UE_LOG(LogTemp, Log, TEXT("Increase statistic"));

		const FString ScoreboardSerialized = FileSubsystem->JsonSerializeStruct(Scoreboard);
		const FString ScoreboardEncoded = FGenericPlatformHttp::UrlEncode(ScoreboardSerialized);

		TSharedRef Request = Http->CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &UDataBaseSubsystem::OnResponseIncreaseStatistics);
		Request->SetURL(ServerURL + "api/users/" + SteamIdEncoded + "/scoreboard/increase");
		Request->SetVerb("POST");

		const FString Content = FString::Printf(TEXT("scoreboardSerialized=%s"), *ScoreboardEncoded);
		Request->SetHeader("Content-Type", "application/x-www-form-urlencoded");
		Request->SetContentAsString(Content);

		const FString AuthHeader = FString::Printf(TEXT("Bearer %s"), *Infos.SessionToken);
		Request->SetHeader("Authorization", AuthHeader);
		Request->ProcessRequest();
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Not logged in to the http server"))
	}
}



void UDataBaseSubsystem::Login()
{
	if (Infos.GuestToken.IsEmpty())
		// Le guestToken n'existe pas, il faut le recreer.
		GetUserGuestToken();

	else
		// Le sessionToken n'existe pas, il faut le recreer.
		CreateSession();
}

void UDataBaseSubsystem::OnPostLogin()
{
	if (!SteamId.IsEmpty()) {
		UE_LOG(LogTemp, Warning, TEXT("Data base post login"));

		bIsLoggedIn = true;
		if(!GetGameInstance()->IsDedicatedServerInstance())
			WriteDataBaseInfos();
	}
}



void UDataBaseSubsystem::VerifyUserState()
{
	UE_LOG(LogTemp, Log, TEXT("Verify User State"));

	TSharedRef Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UDataBaseSubsystem::OnResponseVerifyUserState);
	Request->SetURL(ServerURL + "api/users/" + SteamIdEncoded);
	Request->SetVerb("GET");
	FString AuthHeader = FString::Printf(TEXT("Bearer %s"), *Infos.SessionToken);
	Request->SetHeader("Authorization", AuthHeader);
	Request->ProcessRequest();
}

void UDataBaseSubsystem::OnResponseVerifyUserState(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (ResponseCheck(Response, bWasSuccessful)) {
		UE_LOG(LogTemp, Log, TEXT("User exists"));

		RefreshSession();
	}
	else if (Response->GetResponseCode() != 401) {
		UE_LOG(LogTemp, Warning, TEXT("User doesn't exist, steamId = \"%s\""), *SteamId);

		CreateNewUser();
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("User exists but Session Token has expired"));

		CreateSession();
	}
}



FString UDataBaseSubsystem::GetPath() const noexcept
{
	return FPaths::ProjectDir() + DataDirectory + SteamId + "/" + FileName;
}

void UDataBaseSubsystem::WriteDataBaseInfos() const
{
	FString SerializeInfos = FileSubsystem->JsonSerializeStruct(Infos);

	FileSubsystem->WriteFile(GetPath(), SerializeInfos);
}

void UDataBaseSubsystem::ReadDataBaseInfos()
{
	FString SerializeInfos = FileSubsystem->ReadFile(GetPath());

	if(!SerializeInfos.IsEmpty())
		Infos = FileSubsystem->JsonDeserializeStruct<FUserInfos>(SerializeInfos);
}



void UDataBaseSubsystem::CreateNewUser()
{
	UE_LOG(LogTemp, Log, TEXT("Create New User"));

	FString Name;
	if (const UEfhorisGameInstance* GameInstance = Cast<UEfhorisGameInstance>(GetGameInstance()); ensure(IsValid(GameInstance)))
		Name = GameInstance->GetNickName();

	TSharedRef Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UDataBaseSubsystem::OnResponseCreateNewUser);
	FString SteamIdParam = FString::Printf(TEXT("steamId=%s"), *FGenericPlatformHttp::UrlEncode(SteamId));
	FString NameParam = FString::Printf(TEXT("name=%s"), *FGenericPlatformHttp::UrlEncode(Name));
	FString Params = "?" + SteamIdParam + "&" + NameParam;
	Request->SetURL(ServerURL + "api/users" + Params);
	Request->SetVerb("POST");
	Request->ProcessRequest();
}

void UDataBaseSubsystem::OnResponseCreateNewUser(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (ResponseCheck(Response, bWasSuccessful)) {
		TSharedPtr<FJsonObject> JsonObject = FileSubsystem->JsonDeserialize(Response->GetContentAsString());

		Infos.GuestToken = JsonObject->GetStringField("guestToken");

		UE_LOG(LogTemp, Log, TEXT("User creation successful, guestToken = \"%s\""), *Infos.GuestToken);

		// Creer la session
		CreateSession();
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("User creation failed, steamId = \"%s\""), *SteamId);
}



void UDataBaseSubsystem::GetUserGuestToken()
{
	UE_LOG(LogTemp, Log, TEXT("Get User GuestToken"));

	TSharedRef Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UDataBaseSubsystem::OnResponseGetUserGuestToken);
	Request->SetURL(ServerURL + "api/users/" + SteamIdEncoded + "/profile" );
	Request->SetVerb("GET");
	Request->ProcessRequest();
}

void UDataBaseSubsystem::OnResponseGetUserGuestToken(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (ResponseCheck(Response, bWasSuccessful)) {
		TSharedPtr<FJsonObject> JsonObject = FileSubsystem->JsonDeserialize(Response->GetContentAsString());

		Infos.GuestToken = JsonObject->GetStringField("guestToken");

		UE_LOG(LogTemp, Log, TEXT("Get User's guestToken successful, guestToken = \"%s\""), *Infos.GuestToken);

		if (Infos.SessionToken.IsEmpty())
			CreateSession();
		else
			RefreshSession();
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Get User's guestToken failed, steamId = \"%s\", guestToken = \"%s\""), *SteamId, *Infos.GuestToken);

		// User doesn't exist, create it.
		CreateNewUser();
	}
}



void UDataBaseSubsystem::CreateSession()
{
	UE_LOG(LogTemp, Log, TEXT("Create Session"));

	TSharedRef Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UDataBaseSubsystem::OnResponseCreateSession);
	Request->SetURL(ServerURL + "api/users/" + SteamIdEncoded + "/sessions/create");
	Request->SetVerb("POST");
	FString content = FString::Printf(TEXT("guestToken=%s"), *FGenericPlatformHttp::UrlEncode(Infos.GuestToken));
	Request->SetHeader("Content-Type", "application/x-www-form-urlencoded");
	Request->SetContentAsString(content);
	Request->ProcessRequest();
}

void UDataBaseSubsystem::OnResponseCreateSession(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (ResponseCheck(Response, bWasSuccessful)) {
		Infos.SessionToken = Response->GetContentAsString();

		UE_LOG(LogTemp, Log, TEXT("Session creation successful, sessionToken = \"%s\""), *Infos.SessionToken);

		OnPostLogin();
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Session creation failed, steamId = \"%s\", guestToken = \"%s\""), *SteamId, *Infos.GuestToken);
	
		GetUserGuestToken();
	}
}



void UDataBaseSubsystem::RefreshSession()
{
	UE_LOG(LogTemp, Log, TEXT("Refresh Session"));

	TSharedRef Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UDataBaseSubsystem::OnResponseRefreshSession);
	Request->SetURL(ServerURL + "api/users/" + SteamIdEncoded + "/sessions/refresh");
	Request->SetVerb("POST");
	FString AuthHeader = FString::Printf(TEXT("Bearer %s"), *Infos.SessionToken);
	Request->SetHeader("Authorization", AuthHeader);
	Request->ProcessRequest();
}

void UDataBaseSubsystem::OnResponseRefreshSession(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (ResponseCheck(Response, bWasSuccessful)) {
		Infos.SessionToken = Response->GetContentAsString();

		UE_LOG(LogTemp, Log, TEXT("Session refresh successful, sessionToken = \"%s\""), *Infos.SessionToken);

		OnPostLogin();
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Session refresh failed, sessionToken = \"%s\""), *Infos.SessionToken);

		GetUserGuestToken();
	}
}



void UDataBaseSubsystem::SetUserName(const FString& Name)
{
	UE_LOG(LogTemp, Log, TEXT("Set User Name"));

	TSharedRef Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UDataBaseSubsystem::OnResponseSetUserName);
	FString NameParam = FString::Printf(TEXT("?name=%s"), *FGenericPlatformHttp::UrlEncode(Name));
	Request->SetURL(ServerURL + "api/users/" + SteamIdEncoded + NameParam);
	Request->SetVerb("PATCH");
	FString AuthHeader = FString::Printf(TEXT("Bearer %s"), *Infos.SessionToken);
	Request->SetHeader("Authorization", AuthHeader);
	Request->ProcessRequest();
}

void UDataBaseSubsystem::OnResponseSetUserName(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (ResponseAuthCheck(Request, Response, bWasSuccessful)) {
		TSharedPtr<FJsonObject> JsonObject = FileSubsystem->JsonDeserialize(Response->GetContentAsString());

		UE_LOG(LogTemp, Log, TEXT("Name seted to \"%s\""), *JsonObject->GetStringField("name"));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("Name set unsuccessful"), *Infos.SessionToken);
}



void UDataBaseSubsystem::StoreGameSave(const FString& GameId, const FString& GameSave)
{
	UE_LOG(LogTemp, Log, TEXT("Store Game Save"));

	TSharedRef Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UDataBaseSubsystem::OnResponseStoreGameSave);
	FString GameIdEncoded = FGenericPlatformHttp::UrlEncode(GameId);
	FString GameSaveEncoded = FGenericPlatformHttp::UrlEncode(GameSave);
	Request->SetURL(ServerURL + "api/users/" + SteamIdEncoded + "/saves/" + GameIdEncoded + "/store");
	Request->SetVerb("POST");
	FString content = FString::Printf(TEXT("gameSave=%s"), *GameSaveEncoded);
	Request->SetHeader("Content-Type", "application/x-www-form-urlencoded");
	Request->SetContentAsString(content);
	FString AuthHeader = FString::Printf(TEXT("Bearer %s"), *Infos.SessionToken);
	Request->SetHeader("Authorization", AuthHeader);
	Request->ProcessRequest();
}

void UDataBaseSubsystem::OnResponseStoreGameSave(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (ResponseAuthCheck(Request, Response, bWasSuccessful)) {
		UE_LOG(LogTemp, Log, TEXT("Game Save stored"));

		StoreDirectorSave(DataSave.GameId, DataSave.DirectorSave);
	}
	else
		UE_LOG(LogTemp, Error, TEXT("Game Save store unsuccessful"));
}

void UDataBaseSubsystem::StoreDirectorSave(const FString& GameId, const FString& DirectorSave)
{
	UE_LOG(LogTemp, Log, TEXT("Store Director Save"));


	TSharedRef Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UDataBaseSubsystem::OnResponseStoreDirectorSave);
	FString GameIdEncoded = FGenericPlatformHttp::UrlEncode(GameId);
	FString DirectorSaveEncoded = FGenericPlatformHttp::UrlEncode(DirectorSave);
	Request->SetURL(ServerURL + "api/users/" + SteamIdEncoded + "/saves/" + GameIdEncoded + "/storeAI");
	Request->SetVerb("POST");
	FString content = FString::Printf(TEXT("directorSave=%s"), *DirectorSaveEncoded);
	Request->SetHeader("Content-Type", "application/x-www-form-urlencoded");
	Request->SetContentAsString(content);
	FString AuthHeader = FString::Printf(TEXT("Bearer %s"), *Infos.SessionToken);
	Request->SetHeader("Authorization", AuthHeader);
	Request->ProcessRequest();
}

void UDataBaseSubsystem::OnResponseStoreDirectorSave(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (ResponseAuthCheck(Request, Response, bWasSuccessful)) {
		UE_LOG(LogTemp, Log, TEXT("Director Save stored"));

		for (auto& Pair : DataSave.PlayerSaves) {
			StorePlayerSave(DataSave.GameId, Pair.Key, Pair.Value);
			break;
		}
	}
	else
		UE_LOG(LogTemp, Error, TEXT("Director Save store unsuccessful"));
}

void UDataBaseSubsystem::StorePlayerSave(const FString& GameId, const FString& PlayerId, const FString& PlayerSave)
{
	UE_LOG(LogTemp, Log, TEXT("Store Player Save"));

	TSharedRef Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UDataBaseSubsystem::OnResponseStorePlayerSave);
	FString GameIdEncoded = FGenericPlatformHttp::UrlEncode(GameId);
	FString PlayerIdEncoded = FGenericPlatformHttp::UrlEncode(PlayerId);
	FString PlayerSaveEncoded = FGenericPlatformHttp::UrlEncode(PlayerSave);
	Request->SetURL(ServerURL + "api/users/" + SteamIdEncoded + "/saves/" + GameIdEncoded + "/" + PlayerIdEncoded + "/store");
	Request->SetVerb("POST");
	FString content = FString::Printf(TEXT("playerSave=%s"), *PlayerSaveEncoded);
	Request->SetHeader("Content-Type", "application/x-www-form-urlencoded");
	Request->SetContentAsString(content);
	FString AuthHeader = FString::Printf(TEXT("Bearer %s"), *Infos.SessionToken);
	Request->SetHeader("Authorization", AuthHeader);
	Request->ProcessRequest();
}

void UDataBaseSubsystem::OnResponseStorePlayerSave(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (ResponseAuthCheck(Request, Response, bWasSuccessful)) {
		UE_LOG(LogTemp, Log, TEXT("Player Save stored"));

		FString Key = Response->GetContentAsString();
		DataSave.PlayerSaves.Remove(Key);

		for (auto& Pair : DataSave.PlayerSaves) {
			StorePlayerSave(DataSave.GameId, Pair.Key, Pair.Value);
			break;
		}

		if (DataSave.PlayerSaves.IsEmpty()) {
			USaveSubsystem* SaveSub = GetGameInstance()->GetSubsystem<USaveSubsystem>();

			SaveSub->GetSaveCompleteDelegate().ExecuteIfBound();
		}

	}
	else
		UE_LOG(LogTemp, Error, TEXT("Player Save store unsuccessful"));
}

void UDataBaseSubsystem::OnResponseLoadDirectorSave(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (ResponseAuthCheck(Request, Response, bWasSuccessful)) {
		UE_LOG(LogTemp, Log, TEXT("%s"), *Response->GetContentAsString());
		FDirectorSave DirectorSave = FileSubsystem->JsonDeserializeStruct<FDirectorSave>(Response->GetContentAsString());

		UE_LOG(LogTemp, Log, TEXT("Director save loaded"));

		if (ALobbyPlayerController* PlayerController = GetWorld()->GetFirstPlayerController<ALobbyPlayerController>(); IsValid(PlayerController))
			PlayerController->Server_SetDirectorSave(DirectorSave);
	} else {
		if (ALobbyPlayerController* PlayerController = GetWorld()->GetFirstPlayerController<ALobbyPlayerController>(); IsValid(PlayerController))
			PlayerController->Server_CancelLoading();

		UE_LOG(LogTemp, Error, TEXT("Load director save unsuccessful"));
	}
}

void UDataBaseSubsystem::OnResponseLoadPlayerSaves(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (ResponseAuthCheck(Request, Response, bWasSuccessful)) {
		UE_LOG(LogTemp, Log, TEXT("Player saves loaded"));

		TArray<TSharedPtr<FJsonValue>> JsonValues = FileSubsystem->JsonDeserializeArray(Response->GetContentAsString());

		FPlayerSaves PlayerSaves;
		PlayerSaves.PlayerSaves.Reserve(JsonValues.Num());
		PlayerSaves.PlayersId.Reserve(JsonValues.Num());

		for (auto& Value : JsonValues) {
			FPlayerSave PlayerSave = FileSubsystem->JsonDeserializeStruct<FPlayerSave>(Value->AsString());
			PlayerSaves.Add(PlayerSave.PlayerId, PlayerSave);
		}

		USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();
		SaveSubsystem->StorePlayerStateSaves(PlayerSaves);

		if (ALobbyPlayerController* PlayerController = GetWorld()->GetFirstPlayerController<ALobbyPlayerController>(); IsValid(PlayerController))
			PlayerController->Server_SetPlayerSaves(PlayerSaves);
	} else {
		if (ALobbyPlayerController* PlayerController = GetWorld()->GetFirstPlayerController<ALobbyPlayerController>(); IsValid(PlayerController))
			PlayerController->Server_CancelLoading();

		UE_LOG(LogTemp, Error, TEXT("Load player saves unsuccessful"));
	}
}

void UDataBaseSubsystem::OnResponseDeleteSave(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (ResponseAuthCheck(Request, Response, bWasSuccessful)) {
		UE_LOG(LogTemp, Log, TEXT("Save deleted"));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("Save delete unsuccessful"));
}

void UDataBaseSubsystem::OnResponseGetAllSaves(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (ResponseAuthCheck(Request, Response, bWasSuccessful)) {
		UE_LOG(LogTemp, Log, TEXT("All saves obtained"));
		TArray<TSharedPtr<FJsonValue>> JsonValues = FileSubsystem->JsonDeserializeArray(Response->GetContentAsString());

		TArray<FGameSave> GameSaves;
		GameSaves.Reserve(JsonValues.Num());

		for (auto& Value : JsonValues) {
			FGameSave GameSave = FileSubsystem->JsonDeserializeStruct<FGameSave>(Value->AsString());
			GameSaves.Add(GameSave);
		}

		UE_LOG(LogTemp, Log, TEXT("%d game saves returned by the data base"), GameSaves.Num());

		if (const UEfhorisGameInstance* GameInstance = Cast<UEfhorisGameInstance>(GetGameInstance()); ensure(IsValid(GameInstance)))
			GameInstance->SetAllGameSaveFromDataBase(GameSaves);
	}
	else
		UE_LOG(LogTemp, Error, TEXT("Get all saves unsuccessful"));
}



void UDataBaseSubsystem::OnResponseGetPlayerScoreboard(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (ResponseAuthCheck(Request, Response, bWasSuccessful))
	{
		UE_LOG(LogTemp, Log, TEXT("Player Scoreboard obtained"));

		const FUserScoreboard Scoreboard = FileSubsystem->JsonDeserializeStruct<FUserScoreboard>(Response->GetContentAsString());

		if (const UWorld* World = GetWorld(); IsValid(World))
			if (const ALobbyPlayerController* PlayerController = Cast<ALobbyPlayerController>(World->GetFirstPlayerController()); IsValid(PlayerController))
				PlayerController->SetScoreboard(Scoreboard);
	}
	else
		UE_LOG(LogTemp, Error, TEXT("Get Player Scoreboard unsuccessful"));
}



void UDataBaseSubsystem::OnResponseIncreaseStatistics(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (ResponseAuthCheck(Request, Response, bWasSuccessful))
	{
		UE_LOG(LogTemp, Log, TEXT("Increase statistics successful"));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("Increase statistics unsuccessful"));
}



bool UDataBaseSubsystem::ResponseCheck(FHttpResponsePtr Response, bool bWasSuccessful) const noexcept
{
	int ResponseCode = Response->GetResponseCode();

	if (bWasSuccessful && ResponseCode < 400) {
		UE_LOG(LogTemp, Log, TEXT("Response valid with response code : %d"), ResponseCode);
		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("Response invalid with response code : %d"), ResponseCode);
	return false;
}

bool UDataBaseSubsystem::ResponseAuthCheck(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) noexcept
{
	int ResponseCode = Response->GetResponseCode();
	
	if (ResponseCode == 401) {
		UE_LOG(LogTemp, Warning, TEXT("Response invalid, session token timed out"));
		CreateSession();
		Request->ProcessRequest();
		return false;
	}
	if (bWasSuccessful && ResponseCode < 400) {
		UE_LOG(LogTemp, Verbose, TEXT("Response valid with response code : %d"), ResponseCode);
		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("Response invalid with response code : %d"), ResponseCode);
	return false;
}
