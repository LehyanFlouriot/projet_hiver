#include "AIManager.h"

#include "EfhorisAIController.h"
#include "ProjetHiver/AI/InfluenceMap/InfluenceMap.h"
#include "ProjetHiver/AI/InfluenceMap/InfluenceMapManager.h"
#include "ProjetHiver/AI/InfluenceMap/TemplateManager.h"
#include "ProjetHiver/Characters/BaseCharacter.h"
#include "ProjetHiver/Characters/BaseNPC.h"
#include "ProjetHiver/Characters/DragonBoss.h"
#include "ProjetHiver/Characters/Enemy.h"
#include "ProjetHiver/Characters/Mercenary.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/Contracts/Kevin.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerState.h"

// Sets default values
AAIManager::AAIManager() {
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AAIManager::BeginPlay()
{
	Super::BeginPlay();

	if (IMapManagerClass) {
		InfluenceMapManager = NewObject<UInfluenceMapManager>(NewObject<UInfluenceMapManager>(), IMapManagerClass);
		InfluenceMapManager->Init();
	}
}

void AAIManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (HasAuthority())
	{
		//IMaps
		if (IsValid(InfluenceMapManager)) {
			//Decay all Imaps
			InfluenceMapManager->Tick(DeltaTime);

			//Players influence
			for (auto PlayerState : GetWorld()->GetGameState<AEfhorisGameState>()->PlayerArray) {
				if (const ABaseCharacter* Player = Cast<ABaseCharacter>(PlayerState->GetPawn()); IsValid(Player) && Player->IsAliveHealth()) {
					const FVector Location = PlayerState->GetPawn()->GetActorLocation();
					if (const UInfluenceMap* Template = InfluenceMapManager->GetTemplate(ETemplateType::AllyUnit)) {
						InfluenceMapManager->AddAtCenter(Template, Location,-1.0f);
					}
				}
			}

			//Enemies influence
			for (const auto Enemy : Enemies) {
				if (IsValid(Enemy) && Enemy->IsAliveHealth()) {
					const FVector Location = Enemy->GetActorLocation();
					if (const UInfluenceMap* Template = InfluenceMapManager->GetTemplate(ETemplateType::Enemy)) {
						InfluenceMapManager->AddAtCenter(Template, Location, -1.0f);
					}
				}
			}
		}
	}
	else {
		DrawDebug(DeltaTime);
	}
}

void AAIManager::ClearUnits() {
	for (auto Enemy : Enemies) Enemy->Destroy();
	Enemies.Empty();
	KillMercenaries();
	for (auto Boss : Bosses) Boss->Destroy();
	Bosses.Empty();
}

void AAIManager::AddUnit(ABaseCharacter* Unit) {
	if (IsValid(Unit))
	{
		if (AEnemy* Enemy = Cast<AEnemy>(Unit))
		{
			Enemies.Add(Enemy);
		}
		else if (AMercenary* Mercenary = Cast<AMercenary>(Unit))
		{
			Mercenaries.Add(Mercenary);
		}
		else  if (ADragonBoss* DragonBoss = Cast<ADragonBoss>(Unit))
		{
			Bosses.Add(DragonBoss);
		}
	}
}

void AAIManager::RemoveUnit(ABaseCharacter* Unit) {
	if (IsValid(Unit)) {
		if (AEnemy* Enemy = Cast<AEnemy>(Unit); IsValid(Enemy))
		{
		Enemies.Remove(Enemy);
		}
		else if (AMercenary* Mercenary = Cast<AMercenary>(Unit); IsValid(Mercenary))
		{
			Mercenaries.Remove(Mercenary);
		}
		else  if (ADragonBoss* DragonBoss = Cast<ADragonBoss>(Unit); IsValid(DragonBoss))
		{
			Bosses.Remove(DragonBoss);
		}
	}
}

TArray<AEnemy*>& AAIManager::GetEnemies() {
	return Enemies;
}

void AAIManager::KillEnemies() {
	UE_LOG(LogTemp, Log, TEXT("Killed %i enemies"), Enemies.Num());
	while (!Enemies.IsEmpty()) {
		Enemies[0]->Suicide();
	}
}

void AAIManager::ClearEnemiesThreat(ABaseCharacter* Character) const {
	for (const auto Enemy : Enemies) {
		if (IsValid(Enemy)) {
			Enemy->ClearThreat(Character);
		}
	}
}

TArray<ADragonBoss*>& AAIManager::GetBosses() {
	return Bosses;
}

void AAIManager::KillBosses() {
	UE_LOG(LogTemp, Log, TEXT("Killed %i bosses"), Bosses.Num());
	while (!Bosses.IsEmpty()) {
		Bosses[0]->Suicide();
	}
}

TArray<AMercenary*>& AAIManager::GetMercenaries() {
	return Mercenaries;
}

void AAIManager::KillMercenaries() {
	UE_LOG(LogTemp, Log, TEXT("Killed %i mercenaries"), Mercenaries.Num());
	while (!Mercenaries.IsEmpty()) {
		Mercenaries[0]->Suicide();
	}
}

void AAIManager::ClearMercenariesThreat(ABaseCharacter* Character) const {
	for (AMercenary* Mercenary : Mercenaries) {
		if (IsValid(Mercenary)) {
			Mercenary->ClearThreat(Character);
		}
	}
}

void AAIManager::UpdateBossesScaling(const int NbPlayers) noexcept
{
	for (const ADragonBoss* Boss : Bosses)
		Boss->ScaleBoss(NbPlayers);
}

void AAIManager::DrawDebug(float DeltaTime) const
{
	if (bDebugEnemiesThreat) {
		for (const auto Enemy : Enemies) {
			if (Enemy->HasTarget()) {
				Enemy->DrawDebugThreat(DeltaTime);
			}
		}
	}
	if (bDebugMercenariesThreat) {
		for (const auto Mercenary : Mercenaries) {
			if (IsValid(Mercenary) && Mercenary->IsAliveHealth() && Mercenary->HasTarget()) {
				Mercenary->DrawDebugThreat(DeltaTime);
			}
		}
		
	}
	if (bDebugAllIMaps) {
		InfluenceMapManager->DrawAll(GetWorld(),DeltaTime);
	}
}

void AAIManager::AddUnitsInfluence() const {
	//Enemies
	for (const auto Enemy : Enemies) {
		if (IsValid(Enemy) && Enemy->IsAliveHealth()) {
			const FVector Location = Enemy->GetActorLocation();
			if (const UInfluenceMap* Template = InfluenceMapManager->GetTemplate(ETemplateType::Enemy)) {
				InfluenceMapManager->AddAtCenter(Template, Location, -1.0f);
			}
		}
	}
	//Mercenaries
	for (const auto Mercenary : Mercenaries) {
		if (IsValid(Mercenary) && Mercenary->IsAliveHealth()) {
			const FVector Location = Mercenary->GetActorLocation();
			if (const UInfluenceMap* Template = InfluenceMapManager->GetTemplate(ETemplateType::AllyUnit)) {
				InfluenceMapManager->AddAtCenter(Template, Location, -1.0f);
			}
		}
	}
}

UInfluenceMapManager* AAIManager::GetIMapManager() const {
	return InfluenceMapManager;
}

void AAIManager::IsTargeted(const AEfhorisPlayerState* Player)
{
	for (auto& Enemy : Enemies) {
		if (Enemy->HasTargetId() && !Enemy->HasTarget() && (Enemy->GetTargetId() == Player->GetSteamUserId()))
			if (APlayerCharacter* Player_ = Cast<APlayerCharacter>(Player->GetPawn()); IsValid(Player_))
				Enemy->SetTarget(Player_);
	}
	for (auto& Mercenary : Mercenaries) {
		if (Mercenary->HasMasterId() && !Mercenary->HasMaster() && (Mercenary->GetMasterId() == Player->GetSteamUserId()))
			if (APlayerCharacter* Player_ = Cast<APlayerCharacter>(Player->GetPawn()); IsValid(Player_))
				Mercenary->SetMaster(Player_);
	}

	if (IsValid(Kevin)) {
		if (Kevin->HasTargetId() && !Kevin->HasTarget() && (Kevin->GetTargetId() == Player->GetSteamUserId()))
			if (APlayerCharacter* Player_ = Cast<APlayerCharacter>(Player->GetPawn()); IsValid(Player_))
				Kevin->SetTarget(Player_);
	}
	
}
