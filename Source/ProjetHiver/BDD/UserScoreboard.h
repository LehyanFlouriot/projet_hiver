#pragma once

#include "CoreMinimal.h"
#include "UserScoreboard.generated.h"

USTRUCT(BlueprintType)
struct FUserScoreboard
{
	GENERATED_BODY()

	UPROPERTY()
	int DamageDealt{};

	UPROPERTY()
	int DamageReceived{};

	UPROPERTY()
	int Knockouts{};

	UPROPERTY()
	int Resurrections{};

	UPROPERTY()
	int Deaths{};

	UPROPERTY()
	int PotionsDrank{};

	UPROPERTY()
	int EnemiesKilled{};

	UPROPERTY()
	int BossesKilled{};

	UPROPERTY()
	int GamesWon{};

	UPROPERTY()
	int ItemsBought{};

	UPROPERTY()
	int ItemsSold{};

	UPROPERTY()
	int ItemsPickedUp{};

	UPROPERTY()
	int ItemsUpgraded{};

	UPROPERTY()
	int TilesExplored{};

	UPROPERTY()
	int ContractsCompleted{};
};
