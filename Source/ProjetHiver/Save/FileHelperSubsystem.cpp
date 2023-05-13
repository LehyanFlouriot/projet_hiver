#include "FileHelperSubsystem.h"

void UFileHelperSubsystem::WriteFile(const FString& Path, const FString& Str) const
{
	UE_LOG(LogTemp, Log, TEXT("Write File : %s"), *Path);
	if (FFileHelper::SaveStringToFile(Str, *Path)) {
		UE_LOG(LogTemp, Log, TEXT("Data Saved"));
	} else
		UE_LOG(LogTemp, Error, TEXT("Data Save Unsuccessful"));
}

FString UFileHelperSubsystem::ReadFile(const FString& Path) const
{
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

	if (FileManager.FileExists(*Path)) {
		FString JsonStr;
		if (FFileHelper::LoadFileToString(JsonStr, *Path, FFileHelper::EHashOptions::None)) {
			return JsonStr;
		}
	}
	return {};
}

TArray<FString> UFileHelperSubsystem::FindFiles(const FString& Directory, const FString& FileName) const
{
	UE_LOG(LogTemp, Warning, TEXT("Find Files"));
	TArray<FString> Paths;

	int DotIndex;
	if (!FileName.FindLastChar('.', DotIndex)) {
		UE_LOG(LogTemp, Error, TEXT("The file name \"%s\" has not extension"), *FileName);
		return Paths;
	}

	FString Extension = FileName.Right(FileName.Len() - DotIndex);

	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	FileManager.FindFilesRecursively(Paths, *Directory, *Extension);

	for (int Index = Paths.Num() - 1; Index > -1; --Index)
		if (!Paths[Index].EndsWith(FileName))
			Paths.RemoveAt(Index);

	return Paths;
}

void UFileHelperSubsystem::DeleteDirectory(const FString& Directory) const
{
	UE_LOG(LogTemp, Log, TEXT("Delete File : %s"), *Directory);
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

	if (FileManager.DeleteDirectoryRecursively(*Directory)) {
		UE_LOG(LogTemp, Warning, TEXT("Directory Deleted"));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("Delete Directory Unsuccessful"));
}
