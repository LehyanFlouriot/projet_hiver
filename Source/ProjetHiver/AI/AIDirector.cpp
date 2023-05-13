#include "ProjetHiver/AI/AIDirector.h"

#include "AIManager.h"
#include "Group.h"
#include "Net/UnrealNetwork.h"
#include "ProjetHiver/AI/AIDirectorIntensityRow.h"
#include "ProjetHiver/BDD/UserScoreboardStatistic.h"
#include "ProjetHiver/Characters/BaseCharacter.h"
#include "ProjetHiver/Characters/BaseNPC.h"
#include "ProjetHiver/Characters/DragonBoss.h"
#include "ProjetHiver/Characters/Enemy.h"
#include "ProjetHiver/Characters/Mercenary.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/Characters/Wolf.h"
#include "ProjetHiver/Contracts/AccumulationContractType.h"
#include "ProjetHiver/Contracts/ContractStatus.h"
#include "ProjetHiver/Contracts/Kevin.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameInstance.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerState.h"
#include "ProjetHiver/Inventory/LootChest.h"
#include "ProjetHiver/MapGeneration/Gizmo.h"
#include "ProjetHiver/MapGeneration/LevelGenerator.h"
#include "ProjetHiver/MapGeneration/Tile.h"
#include "ProjetHiver/MapGeneration/Utility/Basics.h"
#include "ProjetHiver/Save/DirectorSave.h"
#include "ProjetHiver/Save/FileHelperSubsystem.h"

AAIDirector::AAIDirector()
{
	PrimaryActorTick.bCanEverTick = true;

	ActorToSpawnInfos.Reserve(300);
}

void AAIDirector::BeginPlay()
{
	Super::BeginPlay();

	if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
		if (AEfhorisGameState* GameState = Cast<AEfhorisGameState>(World->GetGameState()); ensure(IsValid(GameState))) {
			Utilities.SetGameState(GameState);
			AIDirectorFSM.SetGameState(GameState);
			AIDirectorFSM.SetStates(IntensityDataTable->GetRowNames());
		}
}

void AAIDirector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	int Nb = FMath::Min(NbOfActorsToSpawnEachFrame, ActorToSpawnInfos.Num());
	int Size = ActorToSpawnInfos.Num();
	for (int Index = Size - 1; Index > Size - Nb - 1; --Index) {
		UGizmo* Gizmo = ActorToSpawnInfos[Index].Gizmo;
		TSubclassOf<AActor> ActorClass = ActorToSpawnInfos[Index].ActorClass;
		FTransform Transform = ActorToSpawnInfos[Index].Transform;

		Gizmo->SetSpawnedActor(SpawnActorOnGizmo(ActorClass, Transform));

		ActorToSpawnInfos.RemoveAtSwap(Index);
	}
}

void AAIDirector::SetDifficulty(int Difficulty_) noexcept
{
	if (Difficulty_ == -1)
	{
		bCheatDifficultyActivated = false;
		Utilities.Update();
		UpdateDifficulty();
	}
	else
	{
		bCheatDifficultyActivated = true;
		Difficulty = static_cast<EAIDifficulty>(Difficulty_);
	}
}

void AAIDirector::ResetDirector() noexcept {
	LoadedTiles.Empty();
	AIDirectorFSM.ResetState();
}

void AAIDirector::Update(bool bIsFirstUpdate /* = false */)
{
	if (!bIsFirstUpdate) {
		AIDirectorFSM.Update();
		if(!bCheatDifficultyActivated)
		{
			Utilities.Update();
			UpdateDifficulty();
		}
	}
	// Spawn la tuile de start
	else
		SpawnStartTile();

	SpawnGizmos(UpdateLoadedTiles());
}

int AAIDirector::ComputeSpawnRate(EGizmoType Type) const
{
	int SpawnRate = IntensityDataTable->FindRow<FAIDirectorIntensityRow>(AIDirectorFSM.GetCurrentStateName(), "")->SpawnRates[Type];
	if (Type == EGizmoType::AloneEnemy)
		SpawnRate = FMath::RoundToInt(SpawnRate * Utilities.SpawnRateFactor());
	return SpawnRate;
}

void AAIDirector::SpawnGizmos(const TArray<FVector2D>& TilesToLoad)
{
	UE_LOG(LogTemp, Warning, TEXT("CurrentState %s"), *AIDirectorFSM.GetCurrentStateName().ToString());
	UE_LOG(LogTemp, Warning, TEXT("Difficulty : %f"), Utilities.DifficultyUtility());

	for (const FVector2D& Tile : TilesToLoad) {
		UE_LOG(LogTemp, Log, TEXT("Tile %f %f"), Tile.X * LvlGenerator->TileSize.X, Tile.Y * LvlGenerator->TileSize.Y);
		for (EGizmoType Type : TEnumRange<EGizmoType>())
			SpawnGizmoFromTile(Type, Tile);

	}
}

void AAIDirector::SpawnStartTile()
{
	FVector2D StartTile{ 0,0 };
	auto& TileInfo = LvlGenerator->TileLayout[StartTile];
	TileInfo.bIsVisited = true;
	TileInfo.bIsLoaded = true;

	for (EGizmoType Type : TEnumRange<EGizmoType>())
		SpawnGizmoFromTile(Type, StartTile, true);
}

void AAIDirector::SpawnGizmoFromTile(EGizmoType Type, const FVector2D& Tile, bool bIsStartTile /* = false */)
{
	FTileInformation& TileInfo = LvlGenerator->TileLayout[Tile];
	TArray<UGizmo*> Gizmos = TileInfo.Tile->GetGizmos(Type);
	float FloatGizmoToSpawn = Gizmos.Num() * ComputeSpawnRate(Type) / 100.f;
	int NbGizmoToSpawn = FloatGizmoToSpawn < 1.f ?
		(FMath::RandRange(0.f, 1.f) < FloatGizmoToSpawn ? 1.f : 0.f) :
		FMath::RoundToInt(FloatGizmoToSpawn);

	//UEnum* MyEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EGizmoType"));
	//UE_LOG(LogTemp, Log, TEXT("\tNb%sToSpawn : %d"), *MyEnum->GetNameStringByValue(static_cast<int64>(Type)), NbGizmoToSpawn);

	// Choisir aleatoirement les positions ou faire spawn les acteurs
	TArray<int> Indexes;
	Indexes.Reserve(Gizmos.Num());
	for (int i = 0; i < Gizmos.Num(); ++i)
		Indexes.Add(i);
	UniformSelector.Reset(Indexes);

	for (int index : UniformSelector.Pop(NbGizmoToSpawn))
		SpawnGizmo(Gizmos[index], bIsStartTile, Tile);

}

void AAIDirector::SpawnGizmo(UGizmo* Gizmo, bool bIsStartTile, const FVector2D& Tile)
{
	FVector Vect = Gizmo->GetComponentLocation() + FVector(0, 0, 1);

	if (int32 I; GizmosActors.Find({ Gizmo->GetType() }, I)) {
		TArray<TSubclassOf<AWeightedActor>>& Blueprints = GizmosActors[I].Actors;

		// Reset the selector for the good type of gizmo
		RandomSelector.Reset(GizmosActors[I].Actors);

		if (int Size = Blueprints.Num(); Size > 0) {
			// Select an actor using weights
			TSubclassOf<AActor> Class = RandomSelector.Select(Utilities.DifficultyUtility());

			if (IsValid(Cast<ABaseNPC>(Class.GetDefaultObject()))) Vect += FVector(0, 0, 150);

			if(bIsStartTile)
				Gizmo->SetSpawnedActor(SpawnActorOnGizmo(Class, FTransform{ Gizmo->GetComponentRotation(), Vect }));
			else
				ActorToSpawnInfos.Add(FSpawnActorAsynchInfo{ Tile, Gizmo, Class, FTransform{ Gizmo->GetComponentRotation(), Vect } });

			// TODO : Ajouter du debug visuel
		}
	}
}

AActor* AAIDirector::SpawnActorOnGizmo(TSubclassOf<AActor> ActorClass, FTransform Transform) const {
	if (AEfhorisGameMode* GameMode = GetWorld()->GetAuthGameMode<AEfhorisGameMode>()) {
		//Scale NPC (enemy + mercenary)
		if (ActorClass->GetDefaultObject(false)->IsA<ABaseNPC>()) {
			if (ABaseNPC* NPC = GameMode->SpawnActorDeferred<ABaseNPC>(ActorClass, Transform)) {
				NPC->SetCharacterLevel(Difficulty + LvlGenerator->CurrentLevel);
				NPC->FinishSpawning(Transform);
				return NPC;
			}
		}
		//Scale Boss
		else if (ActorClass->GetDefaultObject(false)->IsA<ADragonBoss>()) {
			if (ADragonBoss* Boss = GameMode->SpawnActorDeferred<ADragonBoss>(ActorClass, Transform)) {
				if (const AEfhorisGameState* EfhorisGameState = GetWorld()->GetGameState<AEfhorisGameState>()) {
					Boss->SetCharacterLevel(LvlGenerator->CurrentLevel);
					Boss->FinishSpawning(Transform);
					Boss->ScaleBoss(EfhorisGameState->GetNumberPlayer());
					return Boss;
				}
			}
		}
		//Spawn wolf pack
		else if (ActorClass->GetDefaultObject(false)->IsA<AGroup>()) {
			if (AGroup* Group = GameMode->SpawnActorDeferred<AGroup>(ActorClass, Transform)) {
				Group->SetLevel(Difficulty + LvlGenerator->CurrentLevel);
				Group->SetEnemyCount(2 + AIDirectorFSM.GetIntensity());
				Group->FinishSpawning(Transform);
				return Group;
			}
		}
		else if (AActor* Actor = GameMode->SpawnActor<AActor>(ActorClass, Transform))
			return Actor;
	}

	return nullptr;
}

void AAIDirector::UpdateDifficulty()
{
	const float DifficultyValue = Utilities.DifficultyUtility();
	if (DifficultyValue <= 0.2f)
		Difficulty = EAIDifficulty::Low;
	else if (DifficultyValue > 0.2f && DifficultyValue <= 0.5f)
		Difficulty = EAIDifficulty::Medium;
	else if (DifficultyValue > 0.5f && DifficultyValue <= 0.8f)
		Difficulty = EAIDifficulty::High;
	else
		Difficulty = EAIDifficulty::Insane;
}

TArray<FVector2D> AAIDirector::UpdateLoadedTiles()
{
	UE_LOG(LogTemp, Log, TEXT("UpdateLoadedTiles"));

	AEfhorisGameState* EfhorisGameState = GetWorld()->GetGameState<AEfhorisGameState>();
	ensure(IsValid(EfhorisGameState));

	TArray<FVector2D> TilesToLoad;
	int i = 0;

	for (FVector2D Tile : GetPlayersAdjacentTiles(EfhorisGameState)) {
		if (int j; LoadedTiles.Find(Tile, j)) {
			Swap(LoadedTiles[i], LoadedTiles[j]);
		} else {
			LvlGenerator->TileLayout[Tile].bIsLoaded = true;
			TilesToLoad.Add(Tile);
			LoadedTiles.Insert(Tile,i);
		}
		++i;
	}

	UnloadTiles(i, LoadedTiles.Num() - i);
	
	return TilesToLoad;
}

TArray<FVector2D> AAIDirector::GetPlayersPos(AEfhorisGameState* GameState) const
{
	UE_LOG(LogTemp, Log, TEXT("GetPlayersPos"));

	auto CloseEnough = [](double d1) { return FMath::Abs(d1) < 0.001 ? 0.0 : d1; };

	TArray<FVector2D> PlayersPos;
	for (const auto& PlayerState : GameState->PlayerArray) {
		if (AEfhorisPlayerState* EfhorisPlayerState = Cast<AEfhorisPlayerState>(PlayerState); IsValid(EfhorisPlayerState)) {
			if (APlayerCharacter* Character = Cast<APlayerCharacter>(EfhorisPlayerState->GetPawn()); IsValid(Character)) {
				const FVector& Pos = PlayerState->GetPawn()->GetActorLocation();
				PlayersPos.Add({
					CloseEnough(FMath::RoundHalfToZero(Pos.X / LvlGenerator->TileSize.X)),
					CloseEnough(FMath::RoundHalfToZero(Pos.Y / LvlGenerator->TileSize.Y))
					});

				// Premiere tuile
				if (PlayersPos.Last() == FVector2D(0.0, 0.0)) {
					Character->GetController<AEfhorisPlayerController>()->Client_EnterSafeZone();
				}
				// Deuxieme tuile
				else if (PlayersPos.Last() == FVector2D(0.0, -1.0)) {
					Character->GetController<AEfhorisPlayerController>()->Client_LeftSafeZone();
				}
			}
		}
	}

	if (PlayersPos.IsEmpty())
		PlayersPos.Add(FVector2D{ 0,0 });

	return PlayersPos;
}

TArray<FVector2D> AAIDirector::GetPlayersAdjacentTiles(AEfhorisGameState* GameState) const
{
	UE_LOG(LogTemp, Log, TEXT("GetPlayersAdjacentTiles"));

	TArray<FVector2D> Tiles;

	auto Add = [&](FVector2D& Pos) {
		if (!LvlGenerator->TileLayout[Pos].bIsVisited && !Tiles.Contains(Pos))
			Tiles.Add(Pos);
	};

	for (FVector2D& PlayerPos : GetPlayersPos(GameState)) {
		if (auto& TileInfo = LvlGenerator->TileLayout[PlayerPos]; !TileInfo.bIsVisited) {
			OnNewVisitedTile(TileInfo, GameState);
		}
		for (FVector2D& Pos : LvlGenerator->GetNeighborsPos(PlayerPos)) {
			Add(Pos);
			for (FVector2D& Pos2 : LvlGenerator->GetNeighborsPos(Pos))
				Add(Pos2);
		}
	}

	return Tiles;
}

void AAIDirector::OnNewVisitedTile(FTileInformation& TileInfo, AEfhorisGameState* GameState) const
{
	TileInfo.bIsVisited = true;
	GameState->IncrementNbVisitedTiles();

	for (const auto& PlayerState : GameState->PlayerArray) {
		if (AEfhorisPlayerState* EfhorisPlayerState = Cast<AEfhorisPlayerState>(PlayerState); IsValid(EfhorisPlayerState)) {
			EfhorisPlayerState->IncreaseStatistic(EUserScoreboardStatistic::TilesExplored, 1);
		}
	}

	if (AAccumulationContract* Contract = GameState->GetAccumulationContract(); IsValid(Contract))
		if (Contract->GetType() == EAccumulationContractType::Tile && Contract->GetStatus() == EContractStatus::OnGoing)
			Contract->IncreaseCount();

	if (TileInfo.Type == ETileType::EndTile)
		if (AEfhorisGameMode* GameMode = GetWorld()->GetAuthGameMode<AEfhorisGameMode>(); IsValid(GameMode))
			GameMode->StartBoss();
}

void AAIDirector::UnloadTiles(int Index, int Count)
{
	UE_LOG(LogTemp, Log, TEXT("Unload %d Tiles"), Count);

	// Remove Actors not yet spawned
	for (int i = 0; i < ActorToSpawnInfos.Num();) {
		auto& ActorsInfo = ActorToSpawnInfos[i];

		bool bHasToBeRemoved = false;
		for (int k = Index, j = 0; j < Count; ++k, ++j) {
			if (ActorsInfo.AssociatedTile == LoadedTiles[k]) {
				bHasToBeRemoved = true;
				break;
			}
		}

		if (bHasToBeRemoved) ActorToSpawnInfos.RemoveAtSwap(i);
		else ++i;
	}

	// Remove Actors already spawned
	for (int i = Index, j = 0; j < Count; ++i, ++j) {
		auto& TileInfo = LvlGenerator->TileLayout[LoadedTiles[i]];
		if (!TileInfo.bIsVisited) {
			TileInfo.bIsLoaded = false;
			UE_LOG(LogTemp, Log, TEXT("Unload %f %f Tiles"), LoadedTiles[i].X * LvlGenerator->TileSize.X, LoadedTiles[i].Y * LvlGenerator->TileSize.Y);
			TileInfo.Tile->Unload();
		}
	}
	LoadedTiles.RemoveAt(Index, Count);
}

FGizmosSave AAIDirector::GetGizmosSave(const ATile* Tile, const FVector2D& Tile2D) const
{
	FGizmosSave GizmosSave;

	for (EGizmoType Type : TEnumRange<EGizmoType>()) {
		switch (Type) {
		case EGizmoType::Chest:			GizmosSave.Chests = GetChestsSave(Tile, Tile2D); break;
		case EGizmoType::AloneEnemy:	GizmosSave.Enemies = GetEnemiesSave(Tile, Tile2D); break;
		case EGizmoType::SubBoss:		GizmosSave.SubBosses = GetSubBossesSave(Tile, Tile2D); break;
		case EGizmoType::Boss:			GizmosSave.Bosses = GetBossesSave(Tile, Tile2D); break;
		case EGizmoType::Kevin:			GizmosSave.Kevins = GetKevinsSave(Tile, Tile2D); break;
		case EGizmoType::Mercenary:		GizmosSave.Mercenaries = GetMercenariesSave(Tile, Tile2D); break;
		default:						GizmosSave.OtherGizmos.Append(GetOtherGizmosSave(Tile, Tile2D, Type)); break;
		}
	}
	return GizmosSave;
}

TArray<FChestSave> AAIDirector::GetChestsSave(const ATile* Tile, const FVector2D& Tile2D) const
{
	TArray<FChestSave> ChestsSave;

	for (auto& Giz : Tile->GetGizmos(EGizmoType::Chest)) {
		if (ALootChest* Chest = Cast<ALootChest>(Giz->GetSpawnedActor()); IsValid(Chest)) {
			FChestSave ChestSave;
			ChestSave.bOpened = Chest->IsOpened();
			ChestSave.GizmoName = Giz->GetName();
			ChestSave.Tile = FVector{ Tile2D , 0 };
			ChestsSave.Add(ChestSave);
		}
	}

	return ChestsSave;
}

FEnemySave GetEnemySave(AEnemy* Enemy, UGizmo* Giz, const FVector2D& Tile2D) {
	FEnemySave EnemySave;
	EnemySave.Transform = Enemy->GetActorTransform();
	EnemySave.GizmoName = Giz->GetName();
	EnemySave.Tile = FVector{ Tile2D , 0 };
	EnemySave.HealthPercentage = Enemy->GetMaxHealth() < 0.001f ? 0.1f : Enemy->GetHealth() / Enemy->GetMaxHealth();
	EnemySave.EnemyClassPath = Enemy->GetClass()->GetClassPathName();
	EnemySave.TargetId = Enemy->GetTargetId();

	return EnemySave;
}

TArray<FAloneEnemySave> AAIDirector::GetEnemiesSave(const ATile* Tile, const FVector2D& Tile2D) const
{
	TArray<FAloneEnemySave> EnemiesSave;

	for (auto& Giz : Tile->GetGizmos(EGizmoType::AloneEnemy)) {
		FAloneEnemySave AloneEnemy;
		if (AEnemy* Enemy = Cast<AEnemy>(Giz->GetSpawnedActor()); IsValid(Enemy)) {
			AloneEnemy.Enemy = GetEnemySave(Enemy, Giz, Tile2D);
			EnemiesSave.Add(AloneEnemy);
		} else if (AGroup* Group = Cast<AGroup>(Giz->GetSpawnedActor()); IsValid(Group)) {
			AloneEnemy.bIsGroup = true;
			FGroupSave& GroupSave = AloneEnemy.Group;
			GroupSave.GizmoName = Giz->GetName();
			GroupSave.Tile = FVector{ Tile2D , 0 };
			GroupSave.GroupClassPath = Group->GetClass()->GetClassPathName();
			for (const auto& Wolf : Group->GetUnits()) {
				if (IsValid(Wolf)) {
					GroupSave.Wolfs.Add(GetEnemySave(Wolf, Giz, Tile2D));
				}
			}
			EnemiesSave.Add(AloneEnemy);
		}
	}

	return EnemiesSave;
}

TArray<FEnemySave> AAIDirector::GetSubBossesSave(const ATile* Tile, const FVector2D& Tile2D) const
{
	TArray<FEnemySave> SubBossesSave;

	for (auto& Giz : Tile->GetGizmos(EGizmoType::SubBoss)) {
		if (AEnemy* SubBoss = Cast<AEnemy>(Giz->GetSpawnedActor()); IsValid(SubBoss)) {
			SubBossesSave.Add(GetEnemySave(SubBoss, Giz, Tile2D));
		}
	}

	return SubBossesSave;
}

TArray<FBossSave> AAIDirector::GetBossesSave(const ATile* Tile, const FVector2D& Tile2D) const
{
	TArray<FBossSave> BossesSave;

	for (auto& Giz : Tile->GetGizmos(EGizmoType::Boss)) {
		if (ADragonBoss* Boss = Cast<ADragonBoss>(Giz->GetSpawnedActor()); IsValid(Boss)) {
			FBossSave BossSave;
			BossSave.Transform = Boss->GetActorTransform();
			BossSave.GizmoName = Giz->GetName();
			BossSave.Tile = FVector{ Tile2D , 0 };
			BossSave.HealthPercentage = Boss->GetMaxHealth() < 0.001f ? 0.1f : Boss->GetHealth() / Boss->GetMaxHealth();
			BossSave.Phase = Boss->GetPhase();
			BossesSave.Add(BossSave);
		}
	}

	return BossesSave;
}

TArray<FKevinSave> AAIDirector::GetKevinsSave(const ATile* Tile, const FVector2D& Tile2D) const
{
	TArray<FKevinSave> KevinsSave;

	for (auto& Giz : Tile->GetGizmos(EGizmoType::Kevin)) {
		if (AKevin* Kev = Cast<AKevin>(Giz->GetSpawnedActor()); IsValid(Kev)) {
			FKevinSave KevinSave;
			KevinSave.GizmoName = Giz->GetName();
			KevinSave.Tile = FVector{ Tile2D , 0 };
			KevinSave.Transform = Kev->GetActorTransform();
			KevinSave.TargetId = Kev->GetTargetId();
			KevinsSave.Add(KevinSave);
		}
	}

	return KevinsSave;
}

TArray<FMercenarySave> AAIDirector::GetMercenariesSave(const ATile* Tile, const FVector2D& Tile2D) const
{
	TArray<FMercenarySave> MercenariesSave;

	for (auto& Giz : Tile->GetGizmos(EGizmoType::Mercenary)) {
		if (AMercenary* Mercenary = Cast<AMercenary>(Giz->GetSpawnedActor()); IsValid(Mercenary)) {
			FMercenarySave MercenarySave;
			MercenarySave.Transform = Mercenary->GetActorTransform();
			MercenarySave.GizmoName = Giz->GetName();
			MercenarySave.Tile = FVector{ Tile2D , 0 };
			MercenarySave.HealthPercentage = Mercenary->GetMaxHealth() < 0.001f ? 0.1f : Mercenary->GetHealth() / Mercenary->GetMaxHealth();
			MercenarySave.NbPotionCharges = Mercenary->GetPotionCharge();
			MercenarySave.MasterId = Mercenary->GetMasterId();
			MercenariesSave.Add(MercenarySave);
		}
	}

	return MercenariesSave;
}

TArray<FGizmoSave> AAIDirector::GetOtherGizmosSave(const ATile* Tile, const FVector2D& Tile2D, EGizmoType Type) const
{
	TArray<FGizmoSave> OtherGizmosSave;

	for (auto& Giz : Tile->GetGizmos(Type)) {
		if (IsValid(Giz->GetSpawnedActor())) {
			FGizmoSave OtherGizmoSave;
			OtherGizmoSave.GizmoName = Giz->GetName();
			OtherGizmoSave.Tile = FVector{ Tile2D , 0 };
			OtherGizmoSave.Type = Type;
			OtherGizmosSave.Add(OtherGizmoSave);
		}
	}

	return OtherGizmosSave;
}

FDirectorSave AAIDirector::GetAIDirectorSave() const
{
	FDirectorSave DirectorSave;
	DirectorSave.CurrentNumberOfRoomVisited = AIDirectorFSM.GetCurrentNumberOfRoomVisited();
	DirectorSave.IntensityState = AIDirectorFSM.GetIntensity();
	DirectorSave.CurrentDifficulty = Utilities.DifficultyUtility();

	TArray<FVector> VisitedTiles;
	FGizmosSave GizmosSave;
	for (auto& TileInfo : LvlGenerator->TileLayout) {
		if (TileInfo.Value.bIsVisited) {
			VisitedTiles.Add({ TileInfo.Key.X, TileInfo.Key.Y, 0 });

			GizmosSave.Append(GetGizmosSave(TileInfo.Value.Tile, TileInfo.Key));
		}
	}
	DirectorSave.VisitedTiles = VisitedTiles;
	/*TArray<FVector> LoadedTiles_;
	for (auto& Tile2D : LoadedTiles) {
		LoadedTiles_.Add({ Tile2D.X, Tile2D.Y, 0 });

		ATile* Tile = LvlGenerator->TileLayout[Tile2D].Tile;
		GizmosSave.Append(GetGizmosSave(Tile, Tile2D));
	}
	DirectorSave.LoadedTiles = LoadedTiles_;*/
	DirectorSave.GizmosSave = GizmosSave;

	return DirectorSave;
}



void AAIDirector::SetGizmosSave(FGizmosSave& GizmosSave, const ATile* Tile, const FVector2D& Tile2D)
{
	for (EGizmoType Type : TEnumRange<EGizmoType>()) {
		switch (Type) {
		case EGizmoType::Chest:			SetChestsSave(GizmosSave.Chests, Tile, Tile2D); break;
		case EGizmoType::AloneEnemy:	SetEnemiesSave(GizmosSave.Enemies, Tile, Tile2D); break;
		case EGizmoType::SubBoss:		SetSubBossesSave(GizmosSave.SubBosses, Tile, Tile2D); break;
		case EGizmoType::Boss:			SetBossesSave(GizmosSave.Bosses, Tile, Tile2D); break;
		case EGizmoType::Kevin:			SetKevinsSave(GizmosSave.Kevins, Tile, Tile2D); break;
		case EGizmoType::Mercenary:		SetMercenariesSave(GizmosSave.Mercenaries, Tile, Tile2D); break;
		default:						SetOtherGizmosSave(GizmosSave.OtherGizmos, Tile, Tile2D, Type); break;
		}
	}
}

UGizmo* AAIDirector::FindAssociatedGizmo(const FVector& SaveTile, const FVector2D& Tile2D, const ATile* Tile, const FString& GizmoName, EGizmoType Type) const
{
	FVector2D SaveTile2D{ SaveTile.X, SaveTile.Y };
	if (SaveTile2D == Tile2D) {
		for (auto& Gizmo : Tile->GetGizmos(Type)) {
			if (Gizmo->GetName() == GizmoName)
				return Gizmo;
		}
	}

	return nullptr;
}

void AAIDirector::SetChestsSave(const TArray<FChestSave>& ChestsSave, const ATile* Tile, const FVector2D& Tile2D)
{
	for (const FChestSave& ChestSave : ChestsSave) {
		UGizmo* Gizmo = FindAssociatedGizmo(ChestSave.Tile, Tile2D, Tile, ChestSave.GizmoName, EGizmoType::Chest);
		if (IsValid(Gizmo)) {
			SpawnGizmo(Gizmo, true, Tile2D);
			if(ChestSave.bOpened)
				CastChecked<ALootChest>(Gizmo->GetSpawnedActor())->Open();
		}
	}
}

void AAIDirector::SetEnemiesSave(TArray<FAloneEnemySave>& EnemiesSave, const ATile* Tile, const FVector2D& Tile2D)
{
	UE_LOG(LogTemp, Warning, TEXT("SetEnemiesSave"))
	for (FAloneEnemySave& AloneEnemySave : EnemiesSave) {
		// Spawn Alone Enemy
		if (!AloneEnemySave.bIsGroup) {
			FEnemySave& EnemySave = AloneEnemySave.Enemy;
			UGizmo* Gizmo = FindAssociatedGizmo(EnemySave.Tile, Tile2D, Tile, EnemySave.GizmoName, EGizmoType::AloneEnemy);
			if (IsValid(Gizmo)) {
				FTransform Trans = EnemySave.Transform;
				Trans.SetLocation(Trans.GetLocation() + FVector(0, 0, 150));
				Gizmo->SetSpawnedActor(SpawnActorOnGizmo(FindObject<UClass>(EnemySave.EnemyClassPath), Trans));
				if (AEnemy* Enemy = Cast<AEnemy>(Gizmo->GetSpawnedActor()); IsValid(Enemy)) {
					Enemy->SetHealthPercentage(EnemySave.HealthPercentage);
					Enemy->SetTargetId(EnemySave.TargetId);
				}
			}
		}
		// Spawn Group of enemy
		else {
			FGroupSave& GroupSave = AloneEnemySave.Group;
			UGizmo* Gizmo = FindAssociatedGizmo(GroupSave.Tile, Tile2D, Tile, GroupSave.GizmoName, EGizmoType::AloneEnemy);
			if (IsValid(Gizmo)) {
				FTransform Trans = Gizmo->GetComponentTransform();
				if (AEfhorisGameMode* GameMode = GetWorld()->GetAuthGameMode<AEfhorisGameMode>(); IsValid(GameMode)) {
					if (AGroup* Group = GameMode->SpawnActorDeferred<AGroup>(FindObject<UClass>(GroupSave.GroupClassPath), Trans)) {
						Group->SetLevel(Difficulty + LvlGenerator->CurrentLevel);
						Group->SetEnemyCount(0);
						Group->FinishSpawning(Trans);
						for (auto& WolfSave : GroupSave.Wolfs) {
							Trans = WolfSave.Transform;
							Trans.SetLocation(Trans.GetLocation() + FVector(0, 0, 150));
							AWolf* Wolf = GameMode->SpawnActorDeferred<AWolf>(FindObject<UClass>(WolfSave.EnemyClassPath), Trans);
							Group->AddUnit(Wolf);
							Wolf->FinishSpawning(Trans);
							Wolf->SetTargetId(WolfSave.TargetId);
						}
						Gizmo->SetSpawnedActor(Group);
					}
				}
			}
		}
	}
}

void AAIDirector::SetSubBossesSave(TArray<FEnemySave>& SubBossesSave, const ATile* Tile, const FVector2D& Tile2D)
{
	for (FEnemySave& SubBossSave : SubBossesSave) {
		UGizmo* Gizmo = FindAssociatedGizmo(SubBossSave.Tile, Tile2D, Tile, SubBossSave.GizmoName, EGizmoType::SubBoss);
		if (IsValid(Gizmo)) {
			FTransform Trans = SubBossSave.Transform;
			Trans.SetLocation(Trans.GetLocation() + FVector(0, 0, 150));
			Gizmo->SetSpawnedActor(SpawnActorOnGizmo(FindObject<UClass>(SubBossSave.EnemyClassPath), Trans));
			if (AEnemy* SubBoss = Cast<AEnemy>(Gizmo->GetSpawnedActor()); IsValid(SubBoss)) {
				SubBoss->SetHealthPercentage(SubBossSave.HealthPercentage);
				SubBoss->SetTargetId(SubBossSave.TargetId);
			}
		}
	}
}

void AAIDirector::SetBossesSave(const TArray<FBossSave>& BossesSave, const ATile* Tile, const FVector2D& Tile2D)
{
	for (const FBossSave& BossSave : BossesSave) {
		UGizmo* Gizmo = FindAssociatedGizmo(BossSave.Tile, Tile2D, Tile, BossSave.GizmoName, EGizmoType::Boss);
		if (IsValid(Gizmo)) {
			SpawnGizmo(Gizmo, true, Tile2D);
			if (ADragonBoss* Boss = Cast<ADragonBoss>(Gizmo->GetSpawnedActor()); IsValid(Boss)) {
				Boss->SetActorTransform(BossSave.Transform);
				Boss->SetHealthPercentage(BossSave.HealthPercentage);
				Boss->SetPhase(BossSave.Phase);
			}
		}
	}
}

void AAIDirector::SetKevinsSave(const TArray<FKevinSave>& KevinsSave, const ATile* Tile, const FVector2D& Tile2D)
{
	for (const FKevinSave& KevinSave : KevinsSave) {
		UGizmo* Gizmo = FindAssociatedGizmo(KevinSave.Tile, Tile2D, Tile, KevinSave.GizmoName, EGizmoType::Kevin);
		if (IsValid(Gizmo)) {
			SpawnGizmo(Gizmo, true, Tile2D);
			FTransform Trans = KevinSave.Transform;
			Trans.SetLocation(Trans.GetLocation() + FVector(0, 0, 150));
			if (AKevin* Kev = Cast<AKevin>(Gizmo->GetSpawnedActor()); IsValid(Kev)) {
				Kev->SetActorTransform(Trans);
				Kev->SetTargetId(KevinSave.TargetId);
				Manager->AddKevin(Kev);
			}
		}
	}
}

void AAIDirector::SetMercenariesSave(const TArray<FMercenarySave>& MercenariesSave, const ATile* Tile, const FVector2D& Tile2D)
{
	for (const FMercenarySave& MercenarySave : MercenariesSave) {
		UGizmo* Gizmo = FindAssociatedGizmo(MercenarySave.Tile, Tile2D, Tile, MercenarySave.GizmoName, EGizmoType::Mercenary);
		if (IsValid(Gizmo)) {
			SpawnGizmo(Gizmo, true, Tile2D);
			if (AMercenary* Mercenary = Cast<AMercenary>(Gizmo->GetSpawnedActor()); IsValid(Mercenary)) {
				Mercenary->SetActorTransform(MercenarySave.Transform);
				Mercenary->SetHealthPercentage(MercenarySave.HealthPercentage);
				Mercenary->SetPotionCharge(MercenarySave.NbPotionCharges);
				Mercenary->SetMasterId(MercenarySave.MasterId);
			}
		}
	}
}

void AAIDirector::SetOtherGizmosSave(const TArray<FGizmoSave>& OtherGizmosSave, const ATile* Tile, const FVector2D& Tile2D, EGizmoType Type)
{
	for (const FGizmoSave& OtherGizmoSave : OtherGizmosSave) {
		UGizmo* Gizmo = FindAssociatedGizmo(OtherGizmoSave.Tile, Tile2D, Tile, OtherGizmoSave.GizmoName, Type);
		if (IsValid(Gizmo))
			SpawnGizmo(Gizmo, true, Tile2D);
	}
}


void AAIDirector::SetAIDirectorSave(FDirectorSave& DirectorSave)
{
	AIDirectorFSM.SetNumberOfRoomsVisited(DirectorSave.CurrentNumberOfRoomVisited);
	AIDirectorFSM.SetIntensity(DirectorSave.IntensityState);
	Utilities.SetDifficultyUtility(DirectorSave.CurrentDifficulty);

	for (const FVector& Tile3D : DirectorSave.VisitedTiles) {
		FVector2D Tile2D{ Tile3D.X, Tile3D.Y };
		// Set the tile visited
		LvlGenerator->TileLayout[Tile2D].bIsVisited = true;
		LvlGenerator->TileLayout[Tile2D].bIsLoaded = true;

		auto& Tile = LvlGenerator->TileLayout[Tile2D].Tile;

		SetGizmosSave(DirectorSave.GizmosSave, Tile, Tile2D);
	}
	for (const FVector& Tile3D : DirectorSave.LoadedTiles) {
		FVector2D Tile2D{ Tile3D.X, Tile3D.Y };
		// Set the tile visited
		LvlGenerator->TileLayout[Tile2D].bIsLoaded = true;
		LoadedTiles.Add(Tile2D);

		auto& Tile = LvlGenerator->TileLayout[Tile2D].Tile;

		SetGizmosSave(DirectorSave.GizmosSave, Tile, Tile2D);
	}
}
