#pragma once

class AEfhorisGameState;

/**
 * Efhoris implementation of AI Director Utility Theory.
 * 
 */
class PROJETHIVER_API FAIDirectorUtilityTheory
{
	/** Reference on the GameState */
	AEfhorisGameState* GameState;

	/** 
	 * Current utility Damage Taken.
	 * More the players have taken damage, less the difficulty should be.
	 */
	float UtilityDamageTaken = 0.f;

	/**
	 * Current utility Armor Level.
	 * More the equipment is high level, more the difficulty should be.
	 */
	float UtilityArmorAverageLevel = 0.f;

	/**
	 * Current utility Weapon Level.
	 * More the equipment is high level, more the difficulty should be.
	 */
	float UtilityWeaponAverageLevel = 0.f;

	/**
	 * Current utility Players Alive.
	 * More the number of players alive is great, more the difficulty should be.
	 */
	float UtilityNbPlayersAlive = 0.f;

	/**
	 * Current utility Players Health.
	 * The healthier the players, the harder the game should be.
	 */
	float UtilityPlayersCurrentHealth = 0.f;

	/**
	 * Current utility Number Defeat.
	 * The more the team lost, the easier the game should be.
	 */
	float UtilityNbDefeats = 0.f;

	/** Current utility Difficulty. */
	float UtilityDifficulty = 0.f;

public:
	friend class UEfhorisCheatManager;

	FAIDirectorUtilityTheory() = default;
	~FAIDirectorUtilityTheory() = default;

	FAIDirectorUtilityTheory(const FAIDirectorUtilityTheory&) = delete;
	FAIDirectorUtilityTheory& operator=(const FAIDirectorUtilityTheory&) = delete;

	void SetGameState(AEfhorisGameState* GameState_) noexcept { GameState = GameState_; }

	/**
	 * Computes the utility of each statistics.
	 * Depends on Game State and Player States.
	 */
	void Update();

	/**
	 * Return the current difficulty utility.
	 *
	 * @return current difficulty.
	 */
	FORCEINLINE float DifficultyUtility() const noexcept { return UtilityDifficulty; }

	/**
	 * Return the current difficulty utility of AI Director Utility Theory.
	 * Call `Update` method.
	 *
	 * @return current difficulty.
	 */
	float DifficultyUtilityUpdate() {
		Update();
		return DifficultyUtility();
	}

	/**
	 * Set the difficulty utility.
	 * Used when loading save.
	 * 
	 * @param Diff, the new difficulty
	 */
	void SetDifficultyUtility(float Diff) noexcept { UtilityDifficulty = Diff; }

	/**
	 * Return the utility factor of the spawn rate calculated according to the current difficulty.
	 */
	float SpawnRateFactor() const;

private:
	/** Compute the utility of Damage Taken */
	void ComputeUtilityDamageTaken();

	/** Compute the utility of Equipment Average Level */
	void ComputeUtilityArmorAverageLevel();

	/** Compute the utility of Equipment Average Level */
	void ComputeUtilityWeaponAverageLevel();

	/** Compute the utility of Nb Players Alive */
	void ComputeUtilityNbPlayersAlive();

	/** Compute the utility of Players Current Health */
	void ComputeUtilityPlayersCurrentHealth();

	/** Compute the utility of Number Dafeats */
	void ComputeUtilityNbDefeats();

	/**
	 * Compute the Difficulty.
	 * Difficulty Value shoud be use to define variable for the spawning rates.
	 */
	void ComputeUtilityDifficulty();
};