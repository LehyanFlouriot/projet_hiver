#include "EfhorisPlayerState.h"

#include "GameFramework/SpectatorPawn.h"
#include "Net/UnrealNetwork.h"
#include "ProjetHiver/AI/AIManager.h"
#include "ProjetHiver/BDD/DataBaseSubsystem.h"
#include "ProjetHiver/BDD/UserScoreboardStatistic.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameInstance.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/EfhorisCore/EfhorisHUD.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"
#include "ProjetHiver/MapGeneration/LevelGenerator.h"
#include "ProjetHiver/Save/SaveSubsystem.h"
#include "ProjetHiver/Widgets/GameView.h"

void AEfhorisPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEfhorisPlayerState, SteamNickName);
	DOREPLIFETIME(AEfhorisPlayerState, Scoreboard);
}

void AEfhorisPlayerState::BeginPlay()
{
	Super::BeginPlay();

	Client_GetInformations();
}

void AEfhorisPlayerState::Client_RefreshHealthPanel_Implementation(const TArray<FPlayerInfo>& Descriptions)
{
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetPlayerController()))
		if (AEfhorisHUD* HUD = Cast<AEfhorisHUD>(PlayerController->GetHUD()))
			if (UGameView* GameView = HUD->GetGameView())
				GameView->RefreshBoxHealthTeamPanel(Descriptions);
			
}

void AEfhorisPlayerState::Client_RefreshMapNicknames_Implementation(const TArray<FPlayerInfo>& Descriptions) {
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetPlayerController()))
		if (AEfhorisHUD* HUD = Cast<AEfhorisHUD>(PlayerController->GetHUD()))
			if (UGameView* GameView = HUD->GetGameView())
				GameView->RefreshMapNicknames(Descriptions);
}

FPlayerSave AEfhorisPlayerState::GetPlayerStateSave() const noexcept
{
	FPlayerSave PlayerSave;
	PlayerSave.PlayerId = SteamUserId;

	PlayerSave.Transform = GetPawn()->GetActorTransform();

	const AEfhorisGameState* GameState = GetWorld()->GetGameState<AEfhorisGameState>();
	PlayerSave.CurrentLevel = GameState->GetCurrentLevel();

	if (const APlayerCharacter* Player = GetPawn<APlayerCharacter>(); IsValid(Player))
	{
		PlayerSave.ArmorKit = Player->GetArmorKitInfo();
		PlayerSave.WeaponKit = Player->GetWeaponKitInfo();
		PlayerSave.Belt = Player->GetBeltInfo();
		PlayerSave.Gloves = Player->GetGlovesInfo();
		PlayerSave.Ring = Player->GetRingInfo();
		PlayerSave.Earrings = Player->GetEarringsInfo();
		PlayerSave.Necklace = Player->GetNecklaceInfo();
		PlayerSave.HealthPercentage = Player->GetMaxHealth() < 0.001f ? 0.1f : Player->GetHealth() / Player->GetMaxHealth();
		PlayerSave.NbPotionCharges = Player->GetPotionCharge();
	}
	else {
		PlayerSave.bIsDead = true;
	}

	return PlayerSave;
}

void AEfhorisPlayerState::SetPlayerStateSave(const FPlayerSave& PlayerSave) noexcept
{
	if (APlayerCharacter* Player = GetPawn<APlayerCharacter>(); IsValid(Player))
	{
		const AEfhorisGameState* GameState = GetWorld()->GetGameState<AEfhorisGameState>();
		
		if ((PlayerSave.bIsDead && (PlayerSave.CurrentLevel == GameState->GetCurrentLevel())) || 
			(!PlayerSave.bIsDead)) {
			
			if (PlayerSave.ArmorKit.RowName != FName("None"))
				Player->EquipArmorKit(PlayerSave.ArmorKit);
			else
				Player->UnequipArmorKit(false);
			//
			if (PlayerSave.Belt.RowName != FName("None"))
				Player->EquipBelt(PlayerSave.Belt);
			else
				Player->UnequipBelt();
			//
			if (PlayerSave.Gloves.RowName != FName("None"))
				Player->EquipGloves(PlayerSave.Gloves);
			else
				Player->UnequipGloves();
			//
			if (PlayerSave.Earrings.RowName != FName("None"))
				Player->EquipEarrings(PlayerSave.Earrings);
			else
				Player->UnequipEarrings();
			//
			if (PlayerSave.Ring.RowName != FName("None"))
				Player->EquipRing(PlayerSave.Ring);
			else
				Player->UnequipRing();
			//
			if (PlayerSave.Necklace.RowName != FName("None"))
				Player->EquipNecklace(PlayerSave.Necklace);
			else
				Player->UnequipNecklace();
			//
			if (PlayerSave.WeaponKit.RowName != FName("NoWeapon")) 
				Player->EquipWeaponKit(PlayerSave.WeaponKit);
			else
				Player->UnequipWeaponKit(false);

			if (PlayerSave.bIsDead) {
				Player->SetHealthPercentage(0.f);
				Player->InstantDeath();
			} else {
				if (PlayerSave.CurrentLevel == GameState->GetCurrentLevel()) {
					Player->SetPotionCharge(PlayerSave.NbPotionCharges);
					if (PlayerSave.HealthPercentage < 0.001f) Player->Suicide();
					else Player->SetHealthPercentage(PlayerSave.HealthPercentage);
				}
			}
		}

		if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetPlayerController()); IsValid(PlayerController)) {
			if (PlayerSave.CurrentLevel == GameState->GetCurrentLevel()) {
				FVector Pos = PlayerSave.Transform.GetLocation();
				if (PlayerSave.Transform.GetLocation().Z < 150)
					Pos += FVector{ 0, 0, 300 };

				bool bFirstTile = true;
				if (ALevelGenerator* LvlGen = GetWorld()->GetGameState<AEfhorisGameState>()->GetLevelGenerator(); IsValid(LvlGen)) {
					LvlGen->TileSize.Y;
					bFirstTile = ((FMath::Abs(Pos.X) < (LvlGen->TileSize.X / 2.0)) &&
						(FMath::Abs(Pos.Y) < (LvlGen->TileSize.Y / 2.0)));
				}
				if (bFirstTile) PlayerController->Client_EnterSafeZone();
				else {
					PlayerController->Client_StopAllMusic();
					PlayerController->Client_LeftSafeZone();
				}

				Player->SetActorTransform(FTransform{ PlayerSave.Transform.GetRotation(), Pos });
			}
			else PlayerController->Client_EnterSafeZone();
		}
	}

}

void AEfhorisPlayerState::UploadStatistics() const
{
	if (UDataBaseSubsystem* DBSubsystem = GetGameInstance()->GetSubsystem<UDataBaseSubsystem>(); IsValid(DBSubsystem))
		DBSubsystem->IncreaseStatistics(GetSteamUserId(), Scoreboard);
}

void AEfhorisPlayerState::IncreaseStatistic(EUserScoreboardStatistic ScoreboardStatistic, int Amount)
{
	switch (ScoreboardStatistic)
	{
	case EUserScoreboardStatistic::DamageDealt:
		Scoreboard.DamageDealt += Amount;
		break;
	case EUserScoreboardStatistic::DamageReceived:
		Scoreboard.DamageReceived += Amount;
		break;
	case EUserScoreboardStatistic::Knockouts:
		Scoreboard.Knockouts += Amount;
		break;
	case EUserScoreboardStatistic::Resurrections:
		Scoreboard.Resurrections += Amount;
		break;
	case EUserScoreboardStatistic::Deaths:
		Scoreboard.Deaths += Amount;
		break;
	case EUserScoreboardStatistic::PotionsDrank:
		Scoreboard.PotionsDrank += Amount;
		break;
	case EUserScoreboardStatistic::EnemiesKilled:
		Scoreboard.EnemiesKilled += Amount;
		break;
	case EUserScoreboardStatistic::BossesKilled:
		Scoreboard.BossesKilled += Amount;
		break;
	case EUserScoreboardStatistic::GamesWon:
		Scoreboard.GamesWon += Amount;
		break;
	case EUserScoreboardStatistic::ItemsBought:
		Scoreboard.ItemsBought += Amount;
		break;
	case EUserScoreboardStatistic::ItemsSold:
		Scoreboard.ItemsSold += Amount;
		break;
	case EUserScoreboardStatistic::ItemsPickedUp:
		Scoreboard.ItemsPickedUp += Amount;
		break;
	case EUserScoreboardStatistic::ItemsUpgraded:
		Scoreboard.ItemsUpgraded += Amount;
		break; 
	case EUserScoreboardStatistic::TilesExplored:
		Scoreboard.TilesExplored += Amount;
		break;
	case EUserScoreboardStatistic::ContractsCompleted:
		Scoreboard.ContractsCompleted += Amount;
		break;
	default:
		break;
	}
}

void AEfhorisPlayerState::Client_WriteFilesOnCreator_Implementation(const FFilesToSave& FilesToSave, bool bIsAsynch) const
{
	if (USaveSubsystem* Subsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>(); IsValid(Subsystem)) {
		if(bIsAsynch)
			Subsystem->WriteFilesSaveAsync(FilesToSave);
		else
			Subsystem->WriteStatesSave(FilesToSave);
	}
}

void AEfhorisPlayerState::Client_GetInformations_Implementation()
{
	if (UEfhorisGameInstance* GameInstance = Cast<UEfhorisGameInstance>(GetGameInstance()); IsValid(GameInstance)) {
		FInformations Infos;
		Infos.NickName = GameInstance->GetNickName();
		Infos.UserId = GameInstance->GetPlayerId();
		Infos.bIsCreator = GameInstance->HasCreatedTheSession();

		UDataBaseSubsystem* DataBase = GameInstance->GetSubsystem<UDataBaseSubsystem>();
		Infos.bIsLoggedIn = DataBase->IsLoggedIn();
		Infos.DataBaseInfos = DataBase->GetUserInfos();

		Server_SetInformations(Infos);

		if (Infos.bIsCreator)
			if (const AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetPlayerController()))
				PlayerController->ShowCreatorOptions();
	}
}

void AEfhorisPlayerState::Server_SetInformations_Implementation(const FInformations& Infos)
{
	SteamNickName = Infos.NickName;
	SteamUserId = Infos.UserId;
	bIsCreator = Infos.bIsCreator;

	USaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<USaveSubsystem>();

	bool bIsLoaded = false;
	FPlayerSave Save;
	if (bIsLoaded = SaveSubsystem->GetPlayerSave(SteamUserId, Save); bIsLoaded)
		SetPlayerStateSave(Save);
	else {
		if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetPlayerController()); IsValid(PlayerController))
			PlayerController->Client_EnterSafeZone();
	}

	if (const AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetPlayerController()))
	{
		PlayerController->Authority_UpdateHealthBars();
		PlayerController->Authority_UpdateMapNicknames();
	}

	if (const UWorld* World = GetWorld(); IsValid(World))
		if (AEfhorisGameState* GameState = World->GetGameState<AEfhorisGameState>(); IsValid(GameState)) {
			if (bIsCreator) {
				if (Infos.bIsLoggedIn) {
					if (const UEfhorisGameInstance* GameInstance = GetGameInstance<UEfhorisGameInstance>(); IsValid(GameInstance)) {
						UDataBaseSubsystem* DataBase = GameInstance->GetSubsystem<UDataBaseSubsystem>();

						DataBase->SetUserInfos(Infos.DataBaseInfos);
						DataBase->Init(SteamUserId);
					}
				}
				GameState->SetCreator(this);
			}
			if (AAIManager* AIManager = GameState->GetAIManager(); IsValid(AIManager)) {
				AIManager->UpdateBossesScaling(GameState->GetNumberPlayer());
				if (bIsLoaded) {
					AIManager->IsTargeted(this);
					GameState->IsTargetedByInventory(this);
				}
			}
		}
}