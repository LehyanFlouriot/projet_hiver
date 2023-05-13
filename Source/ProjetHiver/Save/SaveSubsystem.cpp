#include "SaveSubsystem.h"

#include "ProjetHiver/AI/AIDirector.h"
#include "ProjetHiver/BDD/DataBaseSubsystem.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerState.h"
#include "ProjetHiver/LobbyCore/LobbyPlayerController.h"
#include "ProjetHiver/Save/FileHelperSubsystem.h"
#include "Core/Public/HAL/Event.h"


USaveSubsystem::USaveSubsystem()
	: UGameInstanceSubsystem{}, 
	SaveThread{ new FSaveRunnable(&FilesToSaveQueue, &EventSaveAsynch, this) }
{
}

void USaveSubsystem::SaveGame(bool bIsAsynch /* = false */)
{
	UE_LOG(LogTemp, Log, TEXT("Save Game"));

	bool bIsValidSave;
	FFilesToSave FilesToSave = GetFilesToSave(bIsValidSave);
	bIsValidSave &= bIsAsynch ? bIsAutoSaveActivated : true;

	if (bIsValidSave) {
		if (UDataBaseSubsystem* DataBase = GetGameInstance()->GetSubsystem<UDataBaseSubsystem>(); DataBase->IsLoggedIn())
			SaveGameDataBase(FilesToSave, bIsAsynch);
		else
			SaveGameDisk(FilesToSave, bIsAsynch);
	}
}

void USaveSubsystem::LoadGame(const FString& GameId) const
{
	UE_LOG(LogTemp, Log, TEXT("Load Game"));

	if (UDataBaseSubsystem* DataBaseSubSystem = GetGameInstance()->GetSubsystem<UDataBaseSubsystem>(); DataBaseSubSystem->IsLoggedIn()) {
		DataBaseSubSystem->LoadDirectorSave(GameId);
		DataBaseSubSystem->LoadPlayerSaves(GameId);
	} else {
		if (ALobbyPlayerController* PController = GetWorld()->GetFirstPlayerController<ALobbyPlayerController>(); IsValid(PController)) {
			PController->Server_SetDirectorSave(LoadDirectorSave(GameId));
			PController->Server_SetPlayerSaves(LoadPlayerSaves(GameId));
		}
	}
}

const TArray<FGameSave>& USaveSubsystem::GetAllSaves()
{
	UE_LOG(LogTemp, Log, TEXT("Get All Saves"));

	FString Directory = FPaths::ProjectDir() + SaveDirectory;

	const UFileHelperSubsystem* FileSubsystem = GetGameInstance()->GetSubsystem<UFileHelperSubsystem>();

	TArray<FString> SavePaths = FileSubsystem->FindFiles(Directory, GameSaveFileName);

	for (const FString& Path : SavePaths) {
		FGameSave GameSave = ReadStateSave<FGameSave>(Path);

		if (int Index; GameSaves.Find(GameSave, Index)) GameSaves[Index] = GameSave;
		else GameSaves.Add(GameSave);
	}

	GameSaves.Sort();

	return GameSaves;
}

void USaveSubsystem::DeleteSave(const FGameSave& GameSave)
{
	UE_LOG(LogTemp, Log, TEXT("Delete Save"));

	FString Directory = FPaths::ProjectDir() + SaveDirectory + GameSave.GameId.ToString();

	const UFileHelperSubsystem* FileSubsystem = GetGameInstance()->GetSubsystem<UFileHelperSubsystem>();

	FileSubsystem->DeleteDirectory(Directory);

	GameSaves.Remove(GameSave);

	UDataBaseSubsystem* DataBaseSubSystem = GetGameInstance()->GetSubsystem<UDataBaseSubsystem>();

	DataBaseSubSystem->DeleteSave(GameSave.GameId.ToString());
}

void USaveSubsystem::WriteStatesSave(const FFilesToSave& FilesToSave) const
{
	UE_LOG(LogTemp, Log, TEXT("Write States Save"));

	FString DirPath = FPaths::ProjectDir() + SaveDirectory + FilesToSave.GameToSave.State.GameId.ToString() + "/";

	// Write Game Save
	WriteStateSave(DirPath + FilesToSave.GameToSave.FileName, FilesToSave.GameToSave.State);

	// Write Director Save
	WriteStateSave(DirPath + FilesToSave.DirectorToSave.FileName, FilesToSave.DirectorToSave.State);

	// Write Player Saves
	for (const auto& PlayerToSave : FilesToSave.PlayerToSaves) {
		WriteStateSave(DirPath + PlayerToSave.FileName, PlayerToSave.State);
	}

	SaveCompleteDelegate.ExecuteIfBound();
}

void USaveSubsystem::WriteFilesSaveAsync(const FFilesToSave& FilesToSave)
{
	UE_LOG(LogTemp, Warning, TEXT("Write States Save Async"));

	FilesToSaveQueue.Enqueue(FilesToSave);
	SaveThread->SetSaveType(0);
	EventSaveAsynch->Trigger();
}

void USaveSubsystem::StoreStatesSave(const FFilesToSave& FilesToSave) const
{
	UE_LOG(LogTemp, Log, TEXT("Store States Save"))

	UDataBaseSubsystem::FDataBaseSave DataBaseSave;

	const UFileHelperSubsystem* FileSubsystem = GetGameInstance()->GetSubsystem<UFileHelperSubsystem>();

	DataBaseSave.GameId = FilesToSave.GameToSave.State.GameId.ToString();

	// Serialize GameSave
	DataBaseSave.GameSave =
		FileSubsystem->JsonSerializeStruct(FilesToSave.GameToSave.State);

	// Serialize DirectorSave
	DataBaseSave.DirectorSave =
		FileSubsystem->JsonSerializeStruct(FilesToSave.DirectorToSave.State);

	// Serialize PlayerSaves
	for (auto& PlayerToSave : FilesToSave.PlayerToSaves) {
		DataBaseSave.PlayerSaves.Add(PlayerToSave.State.PlayerId,
			FileSubsystem->JsonSerializeStruct(PlayerToSave.State));
	}

	UDataBaseSubsystem* DataBaseSubsystem = GetGameInstance()->GetSubsystem<UDataBaseSubsystem>();

	// Store Saves
	DataBaseSubsystem->StoreSave(DataBaseSave);
}

bool USaveSubsystem::GetPlayerSave(const FString& PlayerId, FPlayerSave& PlayerSave) const
{
	UE_LOG(LogTemp, Log, TEXT("Get Player Save"));

	if (PlayerSaves.Contains(PlayerId)) {
		PlayerSave = PlayerSaves[PlayerId];
		return true;
	}

	return false;
}

void USaveSubsystem::StorePlayerStateSaves(const FPlayerSaves& PlayerSaves_)
{
	UE_LOG(LogTemp, Log, TEXT("Store PlayerState Saves"))

	for (const auto& PlayerId : PlayerSaves_.PlayersId) {
		PlayerSaves.Add(PlayerId, PlayerSaves_.Get(PlayerId));
	}
}

void USaveSubsystem::InitSaverThread() const
{
	SaveThread->Launch();
}

void USaveSubsystem::StopSaverThread() const
{
	UE_LOG(LogTemp, Error, TEXT("Saver Thread stoped"));

	SaveThread->Stop();
	EventSaveAsynch->Trigger();
}

FFilesToSave USaveSubsystem::GetFilesToSave(bool& bCreatorValid)
{
	FFilesToSave FilesToSave;

	if (const AEfhorisGameState* EfhorisGameState = GetWorld()->GetGameState<AEfhorisGameState>(); IsValid(EfhorisGameState)) {
		if (bCreatorValid = IsValid(EfhorisGameState->GetCreator()); bCreatorValid) {
			FGameSave GameSave = EfhorisGameState->GetGameStateSave();

			FilesToSave.GameToSave.State = GameSave;
			FilesToSave.GameToSave.FileName = GameSaveFileName;

			FDirectorSave DirectorSave = EfhorisGameState->GetAIDirector()->GetAIDirectorSave();

			FilesToSave.DirectorToSave.State = DirectorSave;
			FilesToSave.DirectorToSave.FileName = DirectorSaveFileName;

			FilesToSave.PlayerToSaves = GetPlayerStatesToSave(EfhorisGameState);
		}
	}

	return FilesToSave;
}

void USaveSubsystem::SaveGameDisk(const FFilesToSave& FilesToSave, bool bIsAsynch)
{
	UE_LOG(LogTemp, Log, TEXT("Save Game Disk"));

	if (const AEfhorisGameState* EfhorisGameState = GetWorld()->GetGameState<AEfhorisGameState>(); IsValid(EfhorisGameState)) {
		if (IsValid(EfhorisGameState->GetCreator()))
			EfhorisGameState->GetCreator()->Client_WriteFilesOnCreator(FilesToSave, bIsAsynch);
	}
}

void USaveSubsystem::SaveGameDataBase(const FFilesToSave& FilesToSave, bool bIsAsynch)
{
	UE_LOG(LogTemp, Log, TEXT("Save Game Data Base"));

	if (bIsAsynch) {
		FilesToSaveQueue.Enqueue(FilesToSave);
		SaveThread->SetSaveType(1);
		EventSaveAsynch->Trigger();
	}
	else StoreStatesSave(FilesToSave);
}

TArray<FPlayerToSave> USaveSubsystem::GetPlayerStatesToSave(const AEfhorisGameState* EfhorisGameState)
{
	UE_LOG(LogTemp, Log, TEXT("Get PlayerStates To Save"));

	TArray<FPlayerToSave> PlayerToSaves;
	PlayerToSaves.Reserve(EfhorisGameState->PlayerArray.Num());

	for (const auto& Player : EfhorisGameState->PlayerArray) {
		const AEfhorisPlayerState* PlayerState = Cast<AEfhorisPlayerState>(Player);
		ensure(IsValid(PlayerState));

		FPlayerSave PlayerSave = PlayerState->GetPlayerStateSave();

		if (PlayerSaves.Contains(PlayerSave.PlayerId))
			PlayerSaves[PlayerSave.PlayerId] = PlayerSave;
		else PlayerSaves.Add(PlayerSave.PlayerId, PlayerSave);

		FPlayerToSave PlayerToSave;
		PlayerToSave.State = PlayerSave;
		PlayerToSave.FileName = PlayerSave.PlayerId + Extension;
		PlayerToSaves.Add(PlayerToSave);
	}

	return PlayerToSaves;
}

FPlayerSaves USaveSubsystem::LoadPlayerSaves(const FString& GameId) const
{
	UE_LOG(LogTemp, Log, TEXT("Get PlayerState Saves"))

	FPlayerSaves PlayerSaves_;

	const UFileHelperSubsystem* FileSubsystem = GetGameInstance()->GetSubsystem<UFileHelperSubsystem>();

	FString Directory = FPaths::ProjectDir() + SaveDirectory + GameId + "/";

	TArray<FString> PlayerSavePaths = FileSubsystem->FindFiles(Directory, Extension);

	for (FString Path : PlayerSavePaths) {
		if (Path.EndsWith(GameSaveFileName)) break;
		FPlayerSave Save = ReadStateSave<FPlayerSave>(Path);
		PlayerSaves_.Add(Save.PlayerId, Save);
	}

	return PlayerSaves_;
}


FDirectorSave USaveSubsystem::LoadDirectorSave(const FString& GameId) const
{
	UE_LOG(LogTemp, Log, TEXT("Load Director Save"))

	FString DirPath = FPaths::ProjectDir() + SaveDirectory + GameId + "/";

	FString PathDirectorState = DirPath + DirectorSaveFileName;

	return ReadStateSave<FDirectorSave>(PathDirectorState);
}

template<class T>
FString USaveSubsystem::WriteStateSave(const FString& Path, const T& Save) const
{
	UE_LOG(LogTemp, Log, TEXT("Write State Save"));

	const UFileHelperSubsystem* FileSubsystem = GetGameInstance()->GetSubsystem<UFileHelperSubsystem>();

	FString SerializeSave = FileSubsystem->JsonSerializeStruct(Save);

	FileSubsystem->WriteFile(Path, SerializeSave);
	return SerializeSave;
}

template <class T>
T USaveSubsystem::ReadStateSave(const FString& Path) const
{
	UE_LOG(LogTemp, Log, TEXT("Read State Save"));

	const UFileHelperSubsystem* FileSubsystem = GetGameInstance()->GetSubsystem<UFileHelperSubsystem>();

	FString SerializeSave = FileSubsystem->ReadFile(Path);

	return FileSubsystem->JsonDeserializeStruct<T>(SerializeSave);
}
