#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "EfhorisCheatManager.generated.h"

/**
 * Efhoris Cheat Manager
 */
UCLASS()
class PROJETHIVER_API UEfhorisCheatManager : public UCheatManager
{
	GENERATED_BODY()

	/**
	 * TP the Player to the Boss room.
	 */
	UFUNCTION(Exec)
	void Efhoris_TpToBoss() const;

	UFUNCTION(Exec)
	void Efhoris_TpToSpawn() const;

	UFUNCTION(Exec)
	void Efhoris_TpToPlayer(const FString& PlayerNickname) const;

	UFUNCTION(Exec)
	void Efhoris_SpawnBoss() const;

	UFUNCTION(Exec)
	void Efhoris_SpawnEnemy() const;

	UFUNCTION(Exec)
	void Efhoris_SpawnMercenary() const;

	//Todo: move in debug manager
	/** Debug for dataMeasures - Temporary */
	UFUNCTION(Exec)
	void Efhoris_ShowEquipmentStats() const;

	/** Time since level started - Temporary */
	UFUNCTION(Exec)
	void Efhoris_GetTimeSinceLevelStarted() const;

	UFUNCTION(Exec)
	void Efhoris_Suicide() const;

	UFUNCTION(Exec)
	void Efhoris_Lose() const;

	UFUNCTION(Exec)
	void Efhoris_CancelAbilities() const;

	UFUNCTION(Exec)
	void Efhoris_GainMoney(const int Amount = 1000) const;

	UFUNCTION(Exec)
	void Efhoris_MorePotions() const;

	UFUNCTION(Exec)
	void Efhoris_Heal() const;

	UFUNCTION(Exec)
	void Efhoris_InvincibilityToggle() const;

	UFUNCTION(Exec)
	void Efhoris_UltraBuffToggle() const;

	UFUNCTION(Exec)
	void Efhoris_KillEnemies() const;

	UFUNCTION(Exec)
	void Efhoris_KillBosses() const;

	UFUNCTION(Exec)
	void Efhoris_KillMercenaries() const;

	UFUNCTION(Exec)
	void Efhoris_LightsToggle() const;

	/** Rebuild the level */
	UFUNCTION(Exec)
	void Efhoris_RebuildGame(bool bIsGameWon) const;

	/**
	 * Set the intensity of the AI Director
	 * 0 = Rest
	 * 1 = Chill
	 * 2 = Medium
	 * 3 = Intense
	 * -1 = Reset to normal mode
	 */
	UFUNCTION(Exec)
	void Efhoris_SetIntensity(int Intensity) const;

	/**
	 * Set the difficulty of the AI Director
	 * 0 = Low
	 * 1 = Medium
	 * 2 = High
	 * 3 = Insane
	 * -1 = Reset to normal mode
	 */
	UFUNCTION(Exec)
	void Efhoris_SetDifficulty(int Difficulty) const;

	UFUNCTION(Exec)
	void Efhoris_QuitGame() const;

	UFUNCTION(Exec)
	void Efhoris_ShowSignalsToggle() const;
};
