#pragma once

#include "CoreMinimal.h"
#include "PlayerOptions.generated.h"

USTRUCT(BlueprintType)
struct PROJETHIVER_API FPlayerOptions
{
	GENERATED_BODY()

#pragma region Game

protected:

public:

#pragma endregion

#pragma region Video

protected:
	UPROPERTY(BlueprintReadWrite)
	bool bIsFullscreen = false;

public:
	bool GetIsFullscreen() const noexcept;

	void SetIsFullscreen(const bool IsFullscreen) noexcept;

#pragma endregion

#pragma region Audio

protected:
	UPROPERTY(BlueprintReadWrite)
	int MainVolume = 100;

public:
	int GetMainVolume() const noexcept;

	void SetMainVolume(const int NewMainVolume) noexcept;

#pragma endregion
};