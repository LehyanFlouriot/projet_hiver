#include "AIDirectorUtilityTheory.h"

#include "ProjetHiver/AI/Utility/UtilityFunctions.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"

void FAIDirectorUtilityTheory::Update()
{
	ComputeUtilityDamageTaken();
	ComputeUtilityArmorAverageLevel();
	ComputeUtilityWeaponAverageLevel();
	ComputeUtilityNbPlayersAlive();
	ComputeUtilityPlayersCurrentHealth();
	ComputeUtilityDifficulty();
}

float FAIDirectorUtilityTheory::SpawnRateFactor() const
{
	return 0.6f + FMath::LogX(10,UtilityDifficulty + 1.f);
}

void FAIDirectorUtilityTheory::ComputeUtilityDamageTaken()
{
	UtilityDamageTaken = FUtilityFunctions::ExponentielNeg(
		static_cast<int>(GameState->GetDamageTaken()),
		GameState->GetPlayersMaxHealth(),
		1.5f);
}

void FAIDirectorUtilityTheory::ComputeUtilityArmorAverageLevel()
{
	if (GameState->GetAveragePlayersEquipment().AverageArmorLevel - GameState->GetCurrentLevel() <= 0)
		UtilityArmorAverageLevel = 0.f;
	else
		UtilityArmorAverageLevel = FUtilityFunctions::Sigmoid(GameState->GetAveragePlayersEquipment().AverageArmorLevel - GameState->GetCurrentLevel(), 100, 1.f);
}

void FAIDirectorUtilityTheory::ComputeUtilityWeaponAverageLevel()
{
	if (GameState->GetAveragePlayersEquipment().AverageWeaponLevel - GameState->GetCurrentLevel() <= 0)
		UtilityWeaponAverageLevel = 0.f;
	else
		UtilityWeaponAverageLevel = FUtilityFunctions::Sigmoid(GameState->GetAveragePlayersEquipment().AverageWeaponLevel - GameState->GetCurrentLevel(), 100, 1.f);
}

void FAIDirectorUtilityTheory::ComputeUtilityNbPlayersAlive()
{
	if (GameState->GetNumberPlayerAlive() <= 0 || GameState->GetNumberPlayer() <= 0)
		UtilityNbPlayersAlive = 0.f;
	else
		UtilityNbPlayersAlive = FUtilityFunctions::NormalisedLinear(
			GameState->GetNumberPlayerAlive(),
			GameState->GetNumberPlayer()) / GameState->GetNumberPlayer();
}

void FAIDirectorUtilityTheory::ComputeUtilityPlayersCurrentHealth()
{
	const float CurrentHealthPercentage = GameState->GetPlayersCurrentHealth() / GameState->GetPlayersMaxHealth() * 100;
	UtilityPlayersCurrentHealth = FUtilityFunctions::Logit2(CurrentHealthPercentage, 100);
}

void FAIDirectorUtilityTheory::ComputeUtilityNbDefeats()
{
	// TODO
	UtilityPlayersCurrentHealth = 1.f;
}

void FAIDirectorUtilityTheory::ComputeUtilityDifficulty() // TODO : Ameliorer ce calcul, trop simple
{
	UtilityDifficulty = (
		UtilityDamageTaken * 3.f +
		UtilityArmorAverageLevel * 2.f +
		UtilityWeaponAverageLevel * 3.f +
		UtilityNbPlayersAlive * 2.f +
		UtilityPlayersCurrentHealth) / 
		11.f;
}
