#include "PlayerOptions.h"

#pragma region Game

#pragma endregion

#pragma region Video

bool FPlayerOptions::GetIsFullscreen() const noexcept {
	return bIsFullscreen;
}

void FPlayerOptions::SetIsFullscreen(const bool IsFullscreen) noexcept {
	bIsFullscreen = IsFullscreen;
}

#pragma endregion

#pragma region Audio

int FPlayerOptions::GetMainVolume() const noexcept {
	return MainVolume;
}

void FPlayerOptions::SetMainVolume(const int NewMainVolume) noexcept {
	MainVolume = NewMainVolume;
}

#pragma endregion
