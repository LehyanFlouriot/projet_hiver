#include "EfhorisCheatManager.h"

#include "EfhorisGameState.h"
#include "EfhorisPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameInstance.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"
#include "ProjetHiver/MapGeneration/LevelGenerator.h"
#include "ProjetHiver/MapGeneration/Gizmo.h"
#include "ProjetHiver/MapGeneration/Tile.h"
#include "ProjetHiver/AI/AIDirector.h"
#include "ProjetHiver/AI/ActorsByGizmo.h"
#include "ProjetHiver/AI/AIManager.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/Characters/Mercenary.h"
#include "ProjetHiver/Characters/AttributeSets/HealthAttributeSet.h"
#include "ProjetHiver/Widgets/GameView.h"
#include "Engine/DirectionalLight.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjetHiver/Characters/AttributeSets/PotionAttributeSet.h"
#include "ProjetHiver/AI/Signals/SignalComponent.h"


void UEfhorisCheatManager::Efhoris_TpToBoss() const
{
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); ensure(IsValid(PlayerController)))
		if (PlayerController->HasAuthority()) {
			if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
				if (const AEfhorisGameState* GameState = World->GetGameState<AEfhorisGameState>(); IsValid(GameState))
					if (const ALevelGenerator* LevelGenerator = GameState->GetLevelGenerator(); IsValid(LevelGenerator)) {
						FVector2D EndPos = LevelGenerator->EndTilePosition;
						EndPos.X *= LevelGenerator->TileSize.X;
						EndPos.Y *= LevelGenerator->TileSize.Y;
						const FVector NewPos{ EndPos.X, EndPos.Y, 20.0 };
						PlayerController->Authority_TeleportPlayer(NewPos);
						UE_LOG(LogTemp, Warning, TEXT("Has been teleported to %s"), *NewPos.ToString())
					}
		} else
			PlayerController->ServerExec("Efhoris_TpToBoss");
}

void UEfhorisCheatManager::Efhoris_TpToSpawn() const {
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); ensure(IsValid(PlayerController)))
		if (PlayerController->HasAuthority()) {
			if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
				if (const AEfhorisGameState* GameState = World->GetGameState<AEfhorisGameState>(); IsValid(GameState))
					if (const ALevelGenerator* LevelGenerator = GameState->GetLevelGenerator(); IsValid(LevelGenerator)) {
						FVector2D StartPos = LevelGenerator->StartTilePosition;
						StartPos.X *= LevelGenerator->TileSize.X;
						StartPos.Y *= LevelGenerator->TileSize.Y;
						const FVector NewPos{ StartPos.X, StartPos.Y, 20.0 };
						PlayerController->Authority_TeleportPlayer(NewPos);
						UE_LOG(LogTemp, Warning, TEXT("Has been teleported to %s"), *NewPos.ToString())
					}
		}
		else
			PlayerController->ServerExec("Efhoris_TpToSpawn");
}

void UEfhorisCheatManager::Efhoris_TpToPlayer(const FString& PlayerNickname) const {
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); ensure(IsValid(PlayerController)))
		if (PlayerController->HasAuthority()) {
			if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
				if (const AEfhorisGameState* GameState = World->GetGameState<AEfhorisGameState>(); IsValid(GameState))
					for (auto PlayerState : GameState->PlayerArray) {
						if (const auto EPlayerState = Cast<AEfhorisPlayerState>(PlayerState); IsValid(EPlayerState)) {
							if (EPlayerState->GetNickName() == PlayerNickname) {
								if (const APawn* Player = EPlayerState->GetPawn(); IsValid(Player)) {
									const FVector NewPos = Player->GetActorLocation() + FVector(0,0,100);
									PlayerController->Authority_TeleportPlayer(NewPos);
								}
							}
						}
					}
		}
		else
			PlayerController->ServerExec(FString::Printf(TEXT("Efhoris_TpToPlayer %s"), *PlayerNickname));
}

void UEfhorisCheatManager::Efhoris_SpawnBoss() const
{
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); ensure(IsValid(PlayerController)))
		if (PlayerController->HasAuthority()) {
			if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
				if (const AEfhorisGameState* GameState = World->GetGameState<AEfhorisGameState>(); IsValid(GameState))
					if (AAIDirector* AIDirector = GameState->GetAIDirector(); IsValid(AIDirector)) {
						AIDirector->SpawnActorOnGizmo(AIDirector->GizmosActors[AIDirector->GizmosActors.Find({ EGizmoType::Boss })].Actors[0].GetDefaultObject()->ActorClass, PlayerController->GetPawn()->GetTransform());
						UE_LOG(LogTemp, Warning, TEXT("Boss has been spawned to %s"), *PlayerController->GetPawn()->GetTransform().ToString())
					}
		} else
			PlayerController->ServerExec("Efhoris_SpawnBoss");
}

void UEfhorisCheatManager::Efhoris_SpawnEnemy() const {
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); IsValid(PlayerController))
		if (PlayerController->HasAuthority()) {
			if (const UWorld* World = GetWorld(); IsValid(World))
				if (const AEfhorisGameState* GameState = World->GetGameState<AEfhorisGameState>(); IsValid(GameState))
					if (AAIDirector* AIDirector = GameState->GetAIDirector(); IsValid(AIDirector))
						AIDirector->SpawnActorOnGizmo(AIDirector->GizmosActors[AIDirector->GizmosActors.Find({ EGizmoType::AloneEnemy })].Actors[0].GetDefaultObject()->ActorClass, PlayerController->GetPawn()->GetTransform());
		}
		else
			PlayerController->ServerExec("Efhoris_SpawnEnemy");
}

void UEfhorisCheatManager::Efhoris_SpawnMercenary() const {
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); IsValid(PlayerController))
		if (PlayerController->HasAuthority()) {
			if (const UWorld* World = GetWorld(); IsValid(World))
				if (const AEfhorisGameState* GameState = World->GetGameState<AEfhorisGameState>(); IsValid(GameState))
					if (AAIDirector* AIDirector = GameState->GetAIDirector(); IsValid(AIDirector)) {
						AActor* SpawnedActor = AIDirector->SpawnActorOnGizmo(AIDirector->GizmosActors[AIDirector->GizmosActors.Find({ EGizmoType::Mercenary })].Actors[0].GetDefaultObject()->ActorClass, PlayerController->GetPawn()->GetTransform());
						if (AMercenary* Mercenary = Cast<AMercenary>(SpawnedActor); IsValid(Mercenary)) {
							if (APlayerCharacter* Player = Cast<APlayerCharacter>(PlayerController->GetPawn()); IsValid(Player)) {
								Mercenary->SetMaster(Player);
							}
						}
					}
		}
		else
			PlayerController->ServerExec("Efhoris_SpawnMercenary");
}

void UEfhorisCheatManager::Efhoris_ShowEquipmentStats() const
{
	if(const UWorld* World = GetWorld(); ensure(IsValid(World)))
		if(AEfhorisGameState* EfhorisGameState = Cast<AEfhorisGameState>(World->GetGameState()); ensure(IsValid(EfhorisGameState)))
		{
			const FPlayersAverageEquipment PlayersAverageEquipment = EfhorisGameState->GetAveragePlayersEquipment();
			GEngine->AddOnScreenDebugMessage(-1, 50.f, FColor::Blue, "Debug players average equipment level data :");
			//Equipment average level
			GEngine->AddOnScreenDebugMessage(-1, 50.f, FColor::Green, "	Average weapon level : "+FString::SanitizeFloat(PlayersAverageEquipment.AverageWeaponLevel));
			GEngine->AddOnScreenDebugMessage(-1, 50.f, FColor::Green, "	Average armor level : "+FString::SanitizeFloat(PlayersAverageEquipment.AverageArmorLevel));

			GEngine->AddOnScreenDebugMessage(-1, 50.f, FColor::Blue, "Debug players average equipment stats data :");
			//Some attributes for debug purpose
			GEngine->AddOnScreenDebugMessage(-1, 50.f, FColor::Green, "	Average weapon stats physical power : "+FString::SanitizeFloat(PlayersAverageEquipment.AverageWeaponStats.PhysicalPower));
			GEngine->AddOnScreenDebugMessage(-1, 50.f, FColor::Green, "	Average armor stats max health : "+FString::SanitizeFloat(PlayersAverageEquipment.AverageArmorStats.MaxHealth));
		}
}

void UEfhorisCheatManager::Efhoris_GetTimeSinceLevelStarted() const
{
	if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
		if (const AEfhorisGameState* EfhorisGameState = Cast<AEfhorisGameState>(World->GetGameState()); ensure(IsValid(EfhorisGameState)))
		{
			GEngine->AddOnScreenDebugMessage(-1, 50.f, FColor::Red, "Time since level started : " + FString::SanitizeFloat(EfhorisGameState->GetLevelTimer()));
		}
}

void UEfhorisCheatManager::Efhoris_Suicide() const {
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); ensure(IsValid(PlayerController))) {
		if (PlayerController->HasAuthority()) {
			if (const APlayerCharacter* Player = Cast<APlayerCharacter>(PlayerController->GetPawn()); IsValid(Player)) {
				Player->Suicide();
			}
		}
		else {
			PlayerController->ServerExec("Efhoris_Suicide");
		}
	}
}

void UEfhorisCheatManager::Efhoris_Lose() const {
	if (APlayerController* UncastController = GetOuterAPlayerController(); UncastController->HasAuthority()) {
		for (auto PlayerState : GetWorld()->GetGameState()->PlayerArray) {
			if (const APlayerCharacter* Player = Cast<APlayerCharacter>(PlayerState.Get()->GetPawn()); IsValid(Player)) {
				Player->Suicide();
			}
		}
		GetWorld()->GetGameState<AEfhorisGameState>()->GetAIManager()->KillMercenaries();
	}
	else {
		UncastController->ServerExec("Efhoris_Lose");
	}
}

void UEfhorisCheatManager::Efhoris_CancelAbilities() const {
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); IsValid(PlayerController)) {
		if (PlayerController->HasAuthority()) {
			const FGameplayTagContainer CancelTag = FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Character.AbilityEvent.CancelAllActions")));
			if (const APlayerCharacter* Player = PlayerController->GetPawn<APlayerCharacter>(); IsValid(Player)) {
				Player->GetAbilitySystemComponent()->TryActivateAbilitiesByTag(CancelTag);
			}
		}
		else {
			PlayerController->ServerExec("Efhoris_CancelAbilities");
		}
	}
}

void UEfhorisCheatManager::Efhoris_GainMoney(const int Amount) const {
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); IsValid(PlayerController)) {
		if (PlayerController->HasAuthority()) {
			if (const UWorld* World = GetWorld(); IsValid(World)) {
				if (AEfhorisGameState* GameState = World->GetGameState<AEfhorisGameState>(); IsValid(GameState)) {
					GameState->SetMoney(GameState->GetMoney() + Amount);
				}
			}
		}
		else {
			PlayerController->ServerExec(FString::Printf(TEXT("Efhoris_GainMoney %d"), Amount));
		}
	}
}

void UEfhorisCheatManager::Efhoris_MorePotions() const {
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); IsValid(PlayerController)) {
		if (PlayerController->HasAuthority()) {
			if (const APlayerCharacter* Player = PlayerController->GetPawn<APlayerCharacter>(); IsValid(Player)) {
				Player->PotionAttributes->SetPotionCharge(Player->PotionAttributes->GetMaxPotionCharge());
			}
		}
		else {
			PlayerController->ServerExec("Efhoris_MorePotions");
		}
	}
}

void UEfhorisCheatManager::Efhoris_Heal() const {
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); IsValid(PlayerController)) {
		if (PlayerController->HasAuthority()) {
			if (APlayerCharacter* Player = PlayerController->GetPawn<APlayerCharacter>(); IsValid(Player)) {
				Player->HealthAttributes->SetHealth(Player->HealthAttributes->GetMaxHealth());
				if (APlayerCharacter* PlayerHealingMe = Player->PlayerHealingMe; IsValid(PlayerHealingMe))
				{
					PlayerHealingMe->StopInteract(Player);
					PlayerHealingMe->bIsInteracting = false;
				}
				const FGameplayTagContainer DeathTag = FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Character.Die")));
				Player->GetAbilitySystemComponent()->CancelAbilities(&DeathTag);
			}
		}
		else {
			PlayerController->ServerExec("Efhoris_Heal");
		}
	}
}

void UEfhorisCheatManager::Efhoris_InvincibilityToggle() const {
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); IsValid(PlayerController)) {
		if (PlayerController->HasAuthority()) {
			const FGameplayTag InvincibleTag = FGameplayTag::RequestGameplayTag(FName("Character.Status.IsInvincible"));
			if (const APlayerCharacter* Player = PlayerController->GetPawn<APlayerCharacter>(); IsValid(Player)) {
				if (!Player->GetAbilitySystemComponent()->HasMatchingGameplayTag(InvincibleTag))
				{
					Player->GetAbilitySystemComponent()->AddLooseGameplayTag(InvincibleTag);
					Player->GetAbilitySystemComponent()->AddReplicatedLooseGameplayTag(InvincibleTag);
				}
				else
				{
					Player->GetAbilitySystemComponent()->RemoveLooseGameplayTag(InvincibleTag);
					Player->GetAbilitySystemComponent()->RemoveReplicatedLooseGameplayTag(InvincibleTag);
				}
			}
		}
		else {
			PlayerController->ServerExec("Efhoris_InvincibilityToggle");
		}
	}
}

void UEfhorisCheatManager::Efhoris_UltraBuffToggle() const {
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); IsValid(PlayerController)) {
		if (PlayerController->HasAuthority()) {
			const FGameplayTagContainer BuffTag = FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Character.Status.CheatBuff")));
			if (const APlayerCharacter* Player = PlayerController->GetPawn<APlayerCharacter>(); IsValid(Player)) {
				if (!Player->GetAbilitySystemComponent()->HasAnyMatchingGameplayTags(BuffTag)) {
					const FGameplayEffectContextHandle GEBuffContextHandle = Player->GetAbilitySystemComponent()->MakeEffectContext();
					const UGameplayEffect* GEBuffEffect = NewObject<UGameplayEffect>(GetTransientPackage(), PlayerController->CheatBuffEffect);
					Player->GetAbilitySystemComponent()->ApplyGameplayEffectToSelf(GEBuffEffect, 1.0f, GEBuffContextHandle);
				}
				else {
					Player->GetAbilitySystemComponent()->RemoveActiveEffectsWithGrantedTags(BuffTag);
				}
				Player->GetCharacterMovement()->MaxWalkSpeed = Player->GetMaxSpeed();
			}
		}
		else {
			PlayerController->ServerExec("Efhoris_UltraBuffToggle");
		}
	}
}

void UEfhorisCheatManager::Efhoris_KillEnemies() const {
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); IsValid(PlayerController)) {
		if (PlayerController->HasAuthority()) {
			GetWorld()->GetGameState<AEfhorisGameState>()->GetAIManager()->KillEnemies();
		}
		else {
			PlayerController->ServerExec("Efhoris_KillEnemies");
		}
	}
}

void UEfhorisCheatManager::Efhoris_KillBosses() const {
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); IsValid(PlayerController)) {
		if (PlayerController->HasAuthority()) {
			GetWorld()->GetGameState<AEfhorisGameState>()->GetAIManager()->KillBosses();
		}
		else {
			PlayerController->ServerExec("Efhoris_KillBosses");
		}
	}
}

void UEfhorisCheatManager::Efhoris_KillMercenaries() const {
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); IsValid(PlayerController)) {
		if (PlayerController->HasAuthority()) {
			GetWorld()->GetGameState<AEfhorisGameState>()->GetAIManager()->KillMercenaries();
		}
		else {
			PlayerController->ServerExec("Efhoris_KillMercenaries");
		}
	}
}

void UEfhorisCheatManager::Efhoris_LightsToggle() const {
	TArray<AActor*> LightActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), LightActors);
	if (!LightActors.IsEmpty()) {
		if (ADirectionalLight* Light = Cast<ADirectionalLight>(LightActors[0]); IsValid(Light)) {
			if (Light->GetBrightness()<1) {
				Light->SetBrightness(1);
			}
			else {
				Light->SetBrightness(.1f);
			}
		}
	}
}

void UEfhorisCheatManager::Efhoris_RebuildGame(bool bIsGameWon) const
{
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); ensure(IsValid(PlayerController)))
		if (PlayerController->HasAuthority()) {
			if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
				if (AEfhorisGameMode* GameMode = World->GetAuthGameMode<AEfhorisGameMode>(); IsValid(GameMode))
					GameMode->RebuildLevel(bIsGameWon);
		} else
			PlayerController->ServerExec(FString::Printf(TEXT("Efhoris_RebuildGame %d"), bIsGameWon));
}

void UEfhorisCheatManager::Efhoris_SetIntensity(const int Intensity) const
{
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); ensure(IsValid(PlayerController)))
		if (PlayerController->HasAuthority()) {
			if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
				if (const AEfhorisGameState* GameState = World->GetGameState<AEfhorisGameState>(); IsValid(GameState))
					if (const ALevelGenerator* LevelGenerator = GameState->GetLevelGenerator(); IsValid(LevelGenerator))
						if (AAIDirector* AIDirector = GameState->GetAIDirector(); IsValid(AIDirector)) {
							AIDirector->ActorToSpawnInfos.Empty();
							AIDirector->SetIntensity(Intensity);
							for (FVector2D TilePos : AIDirector->LoadedTiles) {
								LevelGenerator->TileLayout[TilePos].Tile->Unload();
							}
							AIDirector->LoadedTiles.Empty();
							AIDirector->Update(false);
						}
		}
		else
			PlayerController->ServerExec(FString::Printf(TEXT("Efhoris_SetIntensity %d"), Intensity));
}

void UEfhorisCheatManager::Efhoris_SetDifficulty(const int Difficulty) const
{
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); ensure(IsValid(PlayerController)))
		if (PlayerController->HasAuthority()) {
			if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
				if (const AEfhorisGameState* GameState = World->GetGameState<AEfhorisGameState>(); IsValid(GameState))
					if (const ALevelGenerator* LevelGenerator = GameState->GetLevelGenerator(); IsValid(LevelGenerator))
						if (AAIDirector* AIDirector = GameState->GetAIDirector(); IsValid(AIDirector)) {
							AIDirector->ActorToSpawnInfos.Empty();
							AIDirector->SetDifficulty(Difficulty);
							switch (Difficulty)
							{
							case 0: AIDirector->Utilities.UtilityDifficulty = 0.f; break;
							case 1: AIDirector->Utilities.UtilityDifficulty = 0.3f; break;
							case 2: AIDirector->Utilities.UtilityDifficulty = 0.6f; break;
							case 3: AIDirector->Utilities.UtilityDifficulty = 0.9f; break;
							}
							
							for (FVector2D TilePos : AIDirector->LoadedTiles) {
								LevelGenerator->TileLayout[TilePos].Tile->Unload();
							}
							AIDirector->LoadedTiles.Empty();
							AIDirector->Update(false);
						}						
		}
		else
			PlayerController->ServerExec(FString::Printf(TEXT("Efhoris_SetDifficulty %d"), Difficulty));
}

void UEfhorisCheatManager::Efhoris_QuitGame() const
{
	if (const AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); ensure(IsValid(PlayerController)))
		PlayerController->QuitGame();
}

void UEfhorisCheatManager::Efhoris_ShowSignalsToggle() const
{
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOuterAPlayerController()); IsValid(PlayerController)) {
		if (PlayerController->HasAuthority()) {
			if (const APlayerCharacter* Player = PlayerController->GetPawn<APlayerCharacter>(); IsValid(Player)) {
				if(USignalComponent* SignalComponent = Cast<USignalComponent>(Player->GetSignalComponent()); IsValid(SignalComponent))
				{
					SignalComponent->SetShowSignals(!SignalComponent->GetShowSignals());
				}
			}
		}
		else {
			PlayerController->ServerExec("Efhoris_ShowSignalsToggle");
		}
	}
}
