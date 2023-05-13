#include "Scoreboard.h"

#include "ScoreboardStatistic.h"
#include "Components/ScrollBox.h"
#include "ProjetHiver/BDD/DataBaseSubsystem.h"
#include "ProjetHiver/BDD/UserScoreboard.h"
#include "ProjetHiver/BDD/UserScoreboardStatistic.h"

void UScoreboard::AddStatistic(const EUserScoreboardStatistic Type, const float Number) {
	if (!IsValid(ScrollBox)) return;
	UScoreboardStatistic* Statistic = Cast<UScoreboardStatistic>(CreateWidget(ScrollBox, StatisticWidgetClass));
	if (!IsValid(Statistic)) return;
	if (const FText* Name = StatisticNames.Find(Type)) {
		Statistic->SetStatisticName(*Name);
		Statistic->SetStatisticNumber(Number);
		ScrollBox->AddChild(Statistic);
	}
}

void UScoreboard::SetScoreboard(const FUserScoreboard& Scoreboard) {
	if (!IsValid(ScrollBox)) return;
	ScrollBox->ClearChildren();
	AddStatistic(EUserScoreboardStatistic::DamageDealt, Scoreboard.DamageDealt);
	AddStatistic(EUserScoreboardStatistic::DamageReceived, Scoreboard.DamageReceived);
	AddStatistic(EUserScoreboardStatistic::Knockouts, Scoreboard.Knockouts);
	AddStatistic(EUserScoreboardStatistic::Resurrections, Scoreboard.Resurrections);
	AddStatistic(EUserScoreboardStatistic::Deaths, Scoreboard.Deaths);
	AddStatistic(EUserScoreboardStatistic::PotionsDrank, Scoreboard.PotionsDrank);
	AddStatistic(EUserScoreboardStatistic::EnemiesKilled, Scoreboard.EnemiesKilled);
	AddStatistic(EUserScoreboardStatistic::BossesKilled, Scoreboard.BossesKilled);
	AddStatistic(EUserScoreboardStatistic::GamesWon, Scoreboard.GamesWon);

	AddStatistic(EUserScoreboardStatistic::ItemsBought, Scoreboard.ItemsBought);
	AddStatistic(EUserScoreboardStatistic::ItemsSold, Scoreboard.ItemsSold);
	AddStatistic(EUserScoreboardStatistic::ItemsPickedUp, Scoreboard.ItemsPickedUp);
	AddStatistic(EUserScoreboardStatistic::ItemsUpgraded, Scoreboard.ItemsUpgraded);

	AddStatistic(EUserScoreboardStatistic::TilesExplored, Scoreboard.TilesExplored);
	AddStatistic(EUserScoreboardStatistic::ContractsCompleted, Scoreboard.ContractsCompleted);
}

void UScoreboard::UpdateScoreboard() const {
	const UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance)) return;
	UDataBaseSubsystem* DataBaseSubsystem = GameInstance->GetSubsystem<UDataBaseSubsystem>();
	if (!IsValid(DataBaseSubsystem)) return;
	DataBaseSubsystem->GetPlayerScoreboard(DataBaseSubsystem->GetSteamId());
}
