#pragma once

#include "CoreMinimal.h"
#include "UserScoreboardStatistic.generated.h"

UENUM(BlueprintType)
enum class EUserScoreboardStatistic : uint8
{
    DamageDealt,
    DamageReceived,
    Knockouts,
    Resurrections,
    Deaths,
    PotionsDrank,
    EnemiesKilled,
    BossesKilled,
    GamesWon,
    ItemsBought,
    ItemsSold,
    ItemsPickedUp,
    ItemsUpgraded,
    TilesExplored,
    ContractsCompleted
};
