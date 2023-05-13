#include "LevelGenerator.h"

#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/MapGeneration/TileLimit.h"
#include "ProjetHiver/MapGeneration/Utility/Basics.h"
#include "ProjetHiver/Contracts/ExplorationContractType.h"


// Sets default values
ALevelGenerator::ALevelGenerator()
{
	bReplicates = true;

	//Initialize the 4 directions of Vector2DToDirection
	Vector2DToDirection.Add(FVector2D(0, -1), EConnection::N);
	Vector2DToDirection.Add(FVector2D(0, 1), EConnection::S);
	Vector2DToDirection.Add(FVector2D(1, 0), EConnection::E);
	Vector2DToDirection.Add(FVector2D(-1, 0), EConnection::W);
	//Initialize the emptyConnections
	EmptyConnections.Add(EConnection::N, false);
	EmptyConnections.Add(EConnection::S, false);
	EmptyConnections.Add(EConnection::E, false);
	EmptyConnections.Add(EConnection::W, false);

	CurrentLevel = 1;
}

ATile* ALevelGenerator::SpawnTile(TSubclassOf<ATile> TileToSpawn, FTransform SpawnTransform) {
	return GetWorld()->SpawnActor<ATile>(TileToSpawn, SpawnTransform);
}

void ALevelGenerator::GenerateTileLayout()
{
	GenerateStartTile();
	for (int TileCount = 0; TileCount < TileAmount; TileCount++)
	{
		if(FindIfShouldStepBack())
		{
			StepBack();
		}
		
		EConnection DesiredDirection = EConnection::Random;
		//South for start tile
		if(TileLayout.Num() <= 1)
		{
			DesiredDirection = EConnection::S;
		}
		FFindNextLocationStruct FindNextLocationResult = FindNextLocation(PreviousLocation, DesiredDirection);
		//Should not be infinite loop
		while (!FindNextLocationResult.HasSucceed)
		{
			StepBack();
			FindNextLocationResult = FindNextLocation(PreviousLocation, DesiredDirection);
		}
		AddNewTileToLayout(FindNextLocationResult.FoundLocation);
	}
	GenerateEndTile();
}

TSubclassOf<ATile> ALevelGenerator::GetTileClass(ETileType TileType, TMap<EConnection, bool> TileConnections)
{
	TArray<TSubclassOf<ATile>> TilePool;
	switch(TileType)
	{
	case ETileType::GenericTile:
		TilePool = GenericTileList ;
		break;
	case ETileType::StartTile:
		return StartTile;
		break;
	case ETileType::EndTile:
		TilePool = EndTileList;
		break;
	case ETileType::SpecialTile:
		TilePool = SpecialTileList;
		break;
	case ETileType::ExplorationContractTile:
		if(const AEfhorisGameState* EfhorisGameState = GetWorld()->GetGameState<AEfhorisGameState>(); ensure(IsValid(EfhorisGameState)))
		{
			uint8 Index = static_cast<uint8>(EfhorisGameState->GetExplorationContract()->GetType());
			TilePool.Append(ExplorationContractTileSets[Index].Tiles);
		}
		break;
	}
	TSubclassOf<ATile> TileToSpawn = FindRandomValidTile(TilePool, TileConnections);
	
	return TileToSpawn;
}

FTransform ALevelGenerator::GetTileTransform(const FVector2D InTileLocation)
{
	const double x = InTileLocation.X * TileSize.X;
	const double y = InTileLocation.Y * TileSize.Y;
	const FVector Location = FVector(x, y, 0);
	FTransform OutTransform = FTransform::Identity;
	OutTransform.SetLocation(Location);
	return OutTransform;
}

FFindNextLocationStruct ALevelGenerator::FindNextLocation(FVector2D StartingLocation, EConnection WantedDirection)
{
	TMap<EConnection, FVector2D> LocationToSeek;
	LocationToSeek.Add(EConnection::N, FVector2D(0, 1));
	LocationToSeek.Add(EConnection::S, FVector2D(0, -1));
	LocationToSeek.Add(EConnection::E, FVector2D(1, 0));
	LocationToSeek.Add(EConnection::W, FVector2D(-1, 0));
	if(WantedDirection == EConnection::Random)
	{
		TArray<EConnection> ToRemove;
		for (TPair< EConnection, FVector2D> Elem : LocationToSeek)
		{
			const FVector2D Location = Elem.Value + StartingLocation;
			if (IsTileUsed(Location))
			{
				ToRemove.Add(Elem.Key);
			}
		}
		for(const EConnection Elem : ToRemove)
		{

			LocationToSeek.Remove(Elem);
		}
		if(LocationToSeek.IsEmpty())
		{
			return FFindNextLocationStruct{ false, FVector2D(0,0) };
		}
		else
		{
			TArray<FVector2D> PossibleLocations;
			for (TPair< EConnection, FVector2D> Elem : LocationToSeek)
			{
				PossibleLocations.Add(Elem.Value);
			}
			const int RandomIndex = Stream.RandRange(0, LocationToSeek.Num()-1);
			const FVector2D RandomPossibleLocation = PossibleLocations[RandomIndex];
			return FFindNextLocationStruct{ true, RandomPossibleLocation + StartingLocation };
		}
	}
	//Used only for StartTile, force return the tile which is in this direction
	else
	{
		return FFindNextLocationStruct{ true, LocationToSeek[WantedDirection] };
	}
}

bool ALevelGenerator::IsTileUsed(const FVector2D Key)
{
	return TileLayout.Find(Key) != nullptr;
}

void ALevelGenerator::ConnectTiles(FVector2D FromTile, FVector2D ToTile)
{
	const FVector2D TileDistance = ToTile - FromTile;
	for (auto Elem : Vector2DToDirection)
	{
		if(Elem.Key == TileDistance)
		{
			SetConnectionEnabled(FromTile, Elem.Value);
			SetConnectionEnabled(ToTile, RevertConnection(Elem.Value));

			// Creation de la limite entre les deux tuiles
			SpawnLimit(FromTile, ToTile);

			return;
		}
	}
}

void ALevelGenerator::SetConnectionEnabled(FVector2D InTile, EConnection DirectionToAdd)
{
	TMap<EConnection, bool> TempDirections = TileLayout.Find(InTile)->Connections;
	if(TempDirections.Contains(DirectionToAdd))
	{
		TempDirections[DirectionToAdd] = true;
	}
	else
	{
		TempDirections.Add(DirectionToAdd, true);
	}
	FTileInformation NewTileInfo = FTileInformation{ TileLayout.Find(InTile)->Type, TempDirections };
	if(TileLayout.Contains(InTile))
	{
		*TileLayout.Find(InTile) = NewTileInfo;
	}
	else
	{
		TileLayout.Add(InTile, NewTileInfo);
	}
}

EConnection ALevelGenerator::RevertConnection(EConnection InConnection)
{
	EConnection RevertedConnection;
	switch (InConnection)
	{
	case EConnection::N:
		RevertedConnection = EConnection::S;
		break;
	case EConnection::S:
		RevertedConnection = EConnection::N;
		break;
	case EConnection::W:
		RevertedConnection = EConnection::E;
		break;
	case EConnection::E:
		RevertedConnection = EConnection::W;
		break;
	case EConnection::Random:
		RevertedConnection = EConnection::N;
		break;
	default:
		RevertedConnection = EConnection::S;
		break;
	}
	return RevertedConnection;
}

bool ALevelGenerator::AreConnectionsMatching(const TMap<EConnection, bool>& InConnections,
	const TMap<EConnection, bool>& AgainstConnections)
{
	bool bTempValidTile = true;
	TArray<EConnection> InConnectionsKeys;
	InConnections.GetKeys(InConnectionsKeys);

	for (const auto InConnectionsKey : InConnectionsKeys)
	{
		const bool* InConnection = InConnections.Find(InConnectionsKey);
		const bool* AgainstConnection = AgainstConnections.Find(InConnectionsKey);
		if(InConnection != nullptr && AgainstConnection != nullptr)
		{
			if(*InConnection != *AgainstConnection)
			{
				bTempValidTile = false;
				break;
			}
		}
	}
	return bTempValidTile;
}

TSubclassOf<ATile> ALevelGenerator::FindRandomValidTile(TArray<TSubclassOf<ATile>> Array,
                                                        TMap<EConnection, bool> inConnections)
{
	TArray<TSubclassOf<ATile>> PossibleTiles;

	for (TSubclassOf<ATile> Elem : Array)
	{
		if(IsValid(Elem))
		{
			if (AreConnectionsMatching(Elem.GetDefaultObject()->TileConnections, inConnections))
			{
				PossibleTiles.Add(Elem);
			}
		}
	}
	if(PossibleTiles.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Possible Tile to spawn not found"));
		return nullptr;
	}

	const int RandomIndex = Stream.RandRange(0, PossibleTiles.Num()-1);
	
	return PossibleTiles[RandomIndex];
}

void ALevelGenerator::GenerateExtraConnections()
{
	if (!TileLayout.IsEmpty()) {
		int NumExtraConnections = 0;
		for (TPair<FVector2D, FTileInformation> Elem : TileLayout)
		{
			TArray<FVector2D> LocationsAround = GetLocationsAround(Elem.Key);
			for (FVector2D Location : LocationsAround)
			{
				if (NumExtraConnections < MaxExtraConnections)
				{
					//---------Gros if----------
					//Si une des deux tuile est la start ou endtile, on annule,
					//Donc on fait des trucs si on a l'inverse de cette condition
					//Autrement dit+
					const auto ElemType = Elem.Value.Type;
					auto LocationType = TileLayout.Find(Location);
					if(LocationType)
					{
						const bool Cond1 = ElemType != ETileType::StartTile;
						const bool Cond2 = ElemType != ETileType::EndTile;
						const bool Cond3 = LocationType->Type != ETileType::StartTile;
						const bool Cond4 = LocationType->Type != ETileType::EndTile;

						if (Cond1 && Cond2 && Cond3 && Cond4)
						{
							if (IsTileUsed(Location))
							{
								//Si les tuiles ne sont pas deja connectees
								if (!AreTilesConnected(Location, Elem.Key))
								{
									const float RandomNumber =Stream.RandRange(0, 100);
									if (RandomNumber <= ChanceForExtraConnections)
									{
										ConnectTiles(Location, Elem.Key);
										UE_LOG(LogTemp, Warning, TEXT("Sucefuly connected two tiles"));
										NumExtraConnections++;
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

TArray<FVector2D> ALevelGenerator::GetLocationsAround(FVector2D inLocation)
{
	TArray<FVector2D> OutLocations;
	for (auto Elem : Vector2DToDirection)
	{
		OutLocations.Add(inLocation + Elem.Key);
	}
	return OutLocations;
}

bool ALevelGenerator::AreTilesConnected(FVector2D inTile, FVector2D inTile2)
{
	const EConnection* DirectionValue = Vector2DToDirection.Find(inTile2 - inTile);
	FTileInformation* TileInformation = TileLayout.Find(inTile);
	if(DirectionValue != nullptr && TileInformation != nullptr)
	{
		const bool* bResultingTileConnection = TileInformation->Connections.Find(*DirectionValue);
		if(bResultingTileConnection != nullptr)
		{
			return *bResultingTileConnection == true;
		}
	}
	return false;
}

void ALevelGenerator::StepBack()
{
	auto PreviousLocationIndex = StepBackHistoric.Find(PreviousLocation);
	if(PreviousLocationIndex != INDEX_NONE)
	{
		PreviousLocationIndex--;
	}
	PreviousLocation = StepBackHistoric[PreviousLocationIndex];
}

void ALevelGenerator::ReadLevelDataFromTable()
{

	TArray<FName> RowNames = LevelsInfoDataTable->GetRowNames();
	for (const FName RowName : RowNames)
	{
		const FLevelInformation* LevelInformation = LevelsInfoDataTable->FindRow<FLevelInformation>(RowName, "");
		if (LevelInformation) {
			if (LevelInformation->CurrentLevel == CurrentLevel)
			{
				//Tiles
				Seed = LevelInformation->Seed;
				TileSize = LevelInformation->TileSize;
				TileAmount = LevelInformation->TileAmount;
				StartTile = LevelInformation->StartTile;
				GenericTileList.Append( LevelInformation->GenericTiles, static_cast<uint8>(ETileDirection::Num) );
				EndTileList.Append( LevelInformation->EndTiles, static_cast<uint8>(ETileDirection::Num));
				SpecialTileList.Append( LevelInformation->SpecialTiles, static_cast<uint8>(ETileDirection::Num));

				ExplorationContractTileSets.Append(LevelInformation->ExplorationContractTileSets, static_cast<uint8>(EExplorationContractType::Num));

				LevelInformation->EndTiles;
				StepBackChance = LevelInformation->StepBackChance;
				ChanceForExtraConnections = LevelInformation->ChanceForExtraConnections;
				MaxExtraConnections = LevelInformation->MaxExtraConnections;
				SpecialTilesAmount = LevelInformation->SpecialTilesAmount;

				//Limit
				TileLimitClass = LevelInformation->TileLimitClass;
			}
		}
	}

}

void ALevelGenerator::ResetLevel(bool bGameWon)
{
	ClearLayout();

	if(bGameWon)
	{
		++CurrentLevel;
	}

	//Retreive level info
	ReadLevelDataFromTable();

	//Generate Tile Layout
	GenerateTiles();
}

void ALevelGenerator::ClearLayout()
{
	for (auto& Info : TileLayout) {
		Info.Value.Tile->Unload();
		Info.Value.Tile->Destroy();
	}
	TileLayout.Empty();
	StepBackHistoric.Empty();
	for (auto& Limit : TilesLimits)
		Limit->Destroy();
	TilesLimits.Empty();
}

void ALevelGenerator::GenerateSpecialTiles()
{
	int SpecialTilesCount = 0;
	TArray<FVector2D> TileLayoutKeysWithoutEdges;

	//Remplissage du tableau
	for (TPair<FVector2D, FTileInformation> Elem : TileLayout)
	{
		TileLayoutKeysWithoutEdges.Add(Elem.Key);
	}

	TileLayoutKeysWithoutEdges.Remove(StartTilePosition);
	TileLayoutKeysWithoutEdges.Remove(EndTilePosition);

	while(!TileLayoutKeysWithoutEdges.IsEmpty() && SpecialTilesCount < SpecialTilesAmount)
	{
		const int RandomIndex = Stream.RandRange(0, TileLayoutKeysWithoutEdges.Num()-1);
		auto CurrentTile = TileLayoutKeysWithoutEdges[RandomIndex];

		const auto [HasSucceeded, FoundLocation] = FindNextLocation(CurrentTile, EConnection::Random);
		if(HasSucceeded)
		{
			TileLayout.Add(FoundLocation, FTileInformation{ ETileType::SpecialTile, EmptyConnections });
			ConnectTiles(CurrentTile, FoundLocation);
			SpecialTilesCount++;
		}
		else
		{
			TileLayoutKeysWithoutEdges.Remove(CurrentTile);
		}
	}
}

void ALevelGenerator::GenerateExplorationContractTile()
{
	TArray<FVector2D> TileLayoutKeysWithoutEdges;
	//Remplissage du tableau
	for (TPair<FVector2D, FTileInformation> Elem : TileLayout)
	{
		TileLayoutKeysWithoutEdges.Add(Elem.Key);
	}

	TileLayoutKeysWithoutEdges.Remove(StartTilePosition);
	TileLayoutKeysWithoutEdges.Remove(EndTilePosition);

	while (!TileLayoutKeysWithoutEdges.IsEmpty())
	{
		const int RandomIndex = Stream.RandRange(0, TileLayoutKeysWithoutEdges.Num() - 1);
		auto CurrentTile = TileLayoutKeysWithoutEdges[RandomIndex];

		const auto [HasSucceed, FoundLocation] = FindNextLocation(CurrentTile, EConnection::Random);
		if (HasSucceed && TileLayout[CurrentTile].Type == ETileType::GenericTile)
		{
			TileLayout.Add(FoundLocation, FTileInformation{ ETileType::ExplorationContractTile, EmptyConnections });
			ConnectTiles(CurrentTile, FoundLocation);
			return;
		}
	}
}

void ALevelGenerator::GenerateStartTile()
{
	FTileInformation StartTileInformation{ ETileType::StartTile, EmptyConnections };
	FVector2D InitialPosition = FVector2D(0, 0);
	TileLayout.Add(InitialPosition, StartTileInformation);
	StartTilePosition = InitialPosition;
	StepBackHistoric.Add(InitialPosition);
	PreviousLocation = InitialPosition;
}

void ALevelGenerator::GenerateEndTile()
{
	FFindNextLocationStruct FindNextLocationResult = FindNextLocation(PreviousLocation, EConnection::Random);
	while(!FindNextLocationResult.HasSucceed)
	{
		UE_LOG(LogTemp, Warning, TEXT("Location not founded, stepping back in GenerateEndTile"));
		StepBack();
		FindNextLocationResult = FindNextLocation(PreviousLocation, EConnection::Random);
	}
	TileLayout.Add(FindNextLocationResult.FoundLocation, FTileInformation{ ETileType::EndTile, EmptyConnections });
	EndTilePosition = FindNextLocationResult.FoundLocation;
	StepBackHistoric.Add(FindNextLocationResult.FoundLocation);
	ConnectTiles(PreviousLocation, FindNextLocationResult.FoundLocation);
}

bool ALevelGenerator::FindIfShouldStepBack()
{
	const int32 PreviousIndex = StepBackHistoric.Find(PreviousLocation);
	if(PreviousIndex - 1 != -1)
	{
		const int32 RandNumber =Stream.RandRange(0, 100);
		if(StepBackHistoric[PreviousIndex - 1] != FVector2D(0,0) 
			&& RandNumber <= StepBackChance)
		{
			return true;
		}
	}
	return false;
}

void ALevelGenerator::AddNewTileToLayout(FVector2D FoundLocationInput)
{
	TileLayout.Add(FoundLocationInput, FTileInformation{ ETileType::GenericTile , EmptyConnections});
	StepBackHistoric.Add(FoundLocationInput);
	ConnectTiles(PreviousLocation, FoundLocationInput);
	PreviousLocation = FoundLocationInput;
}

void ALevelGenerator::GenerateTiles()
{
	GenerateStream();
	GenerateTileLayout();
	GenerateExtraConnections();
	GenerateSpecialTiles();
	GenerateExplorationContractTile();
	SpawnTiles();
}

void ALevelGenerator::SpawnTiles()
{
	for (TPair<FVector2D, FTileInformation>& Elem : TileLayout)
	{
		FTileSpawnInformation TileInfo{ GetTileClass(Elem.Value.Type, Elem.Value.Connections), GetTileTransform(Elem.Key) };
		Elem.Value.Tile = SpawnTile(TileInfo.TileToSpawn, TileInfo.Transform);
	}
}

void ALevelGenerator::GenerateStream()
{
	Stream = FRandomStream{ Seed };
}

TArray<FLocatedTile> ALevelGenerator::GenerateArrayFromLayoutMap() {
	TArray<FLocatedTile> ArrayLayout;
	for (const auto& TileTuple : TileLayout)
	{
		FLocatedTile LocatedTile;
		LocatedTile.Position = TileTuple.Key;
		LocatedTile.Texture = TileTuple.Value.Tile->GetTexture();
		ArrayLayout.Add(LocatedTile);
	}
	return ArrayLayout;
}

TArray<FVector2D> ALevelGenerator::GetNeighborsPos(FVector2D TilePos)
{
	TArray<FVector2D> Neighbors;
	if (TileLayout.Contains(TilePos))
		for (const auto& Connection : TileLayout[TilePos].Connections)
			if(Connection.Value)
				switch (Connection.Key) {
				case EConnection::N : Neighbors.Add(TilePos + FVector2D(0, -1)); break;
				case EConnection::S : Neighbors.Add(TilePos + FVector2D(0, 1)); break;
				case EConnection::E : Neighbors.Add(TilePos + FVector2D(1, 0)); break;
				case EConnection::W : Neighbors.Add(TilePos + FVector2D(-1, 0)); break;
				default: break;
				}

	return Neighbors;
}

TArray<EConnection> ALevelGenerator::GetConnections(FVector2D TilePos) {
	TArray<EConnection> Connections;
	if (TileLayout.Contains(TilePos))
		for (const auto& Connection : TileLayout[TilePos].Connections)
			if (Connection.Value)
				Connections.Add(Connection.Key);

	return Connections;
}

FVector ALevelGenerator::GetConnectionWorldLocation(FVector2D TilePos, EConnection Connection) {
	const FVector TileLocation = GetTileTransform(TilePos).GetLocation();
	FVector ConnectionOffset;
	switch (Connection)
	{
	case EConnection::N:
		ConnectionOffset = FVector(TileSize.X/5, -TileSize.X/2, 0);
		break;
	case EConnection::E:
		ConnectionOffset = FVector(TileSize.Y/2, TileSize.Y/6, 0);
		break;
	case EConnection::W:
		ConnectionOffset = FVector(-TileSize.Y/2, TileSize.Y/6, 0);
		break;
	case EConnection::S:
		ConnectionOffset = FVector(TileSize.X/5, TileSize.X/2, 0);
		break;
	default:
		break;
	}
	return TileLocation + ConnectionOffset;
}

void ALevelGenerator::SpawnLimit(FVector2D FromTile, FVector2D ToTile)
{
	if (HasAuthority())
		if (UWorld* World = GetWorld(); ensure(IsValid(World)))
			if (ATileLimit* TileLimit = World->SpawnActor<ATileLimit>(TileLimitClass, FTransform{ FVector(1000,1000,1000) }); ensure(IsValid(TileLimit)))
				TilesLimits.Add(TileLimit), TileLimit->SetDirection(FromTile, ToTile);
}
