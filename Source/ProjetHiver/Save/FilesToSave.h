#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Save/DirectorSave.h"
#include "ProjetHiver/Save/GameSave.h"
#include "ProjetHiver/Save/PlayerSave.h"
#include "FilesToSave.generated.h"

USTRUCT()
struct PROJETHIVER_API FGameToSave {
	GENERATED_BODY()

	UPROPERTY()
	FString FileName {};

	UPROPERTY()
	FGameSave State {};
};

USTRUCT()
struct PROJETHIVER_API FDirectorToSave {
	GENERATED_BODY()

	UPROPERTY()
	FString FileName {};

	UPROPERTY()
	FDirectorSave State {};
};

USTRUCT()
struct PROJETHIVER_API FPlayerToSave {
	GENERATED_BODY()

	UPROPERTY()
	FString FileName {};

	UPROPERTY()
	FPlayerSave State {};
};

USTRUCT()
struct PROJETHIVER_API FFilesToSave {
	GENERATED_BODY()

	UPROPERTY()
	FGameToSave GameToSave {};

	UPROPERTY()
	FDirectorToSave DirectorToSave {};

	UPROPERTY()
	TArray<FPlayerToSave> PlayerToSaves {};
};
