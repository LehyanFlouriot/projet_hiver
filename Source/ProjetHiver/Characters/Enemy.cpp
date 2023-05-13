#include "Enemy.h"

#include "GameFramework/PawnMovementComponent.h"
#include "Components/SphereComponent.h"
#include "ProjetHiver/AI/AIManager.h"
#include "ProjetHiver/AI/InfluenceMap/InfluenceMap.h"
#include "ProjetHiver/AI/InfluenceMap/InfluenceMapManager.h"
#include "ProjetHiver/AI/InfluenceMap/TemplateManager.h"
#include "ProjetHiver/Characters/Mercenary.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/Contracts/AccumulationContractType.h"
#include "ProjetHiver/Contracts/ContractStatus.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"
#include "ProjetHiver/LootComponent/LootComponent.h"

AEnemy::AEnemy(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	LootComponent = CreateDefaultSubobject<ULootComponent>(TEXT("LootComponent"));
	LootComponent->SetupAttachment(RootComponent);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(RootComponent);

	SphereComponent->SetSphereRadius(500.f);
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnSphereBeginOverlap);
}

void AEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SphereComponent->OnComponentBeginOverlap.RemoveDynamic(this, &AEnemy::OnSphereBeginOverlap);
}

void AEnemy::Die()
{
	Super::Die();

	LootComponent->GenerateLoot();

	GetMovementComponent()->Velocity = {};

	if (const AEfhorisGameState* GameState = GetWorld()->GetGameState<AEfhorisGameState>(); IsValid(GameState))
		if (AAccumulationContract* Contract = GameState->GetAccumulationContract(); IsValid(Contract))
			if (Contract->GetType() == EAccumulationContractType::Kill && Contract->GetStatus() == EContractStatus::OnGoing)
				Contract->IncreaseCount();
}

void AEnemy::ClearUnitReferences()
{
	if(HasAuthority())
	{

		//for (const TPair<ABaseCharacter*, int>& Pair : ThreatMap)
		//{
		//	if (!IsValid(Pair.Key) || !Pair.Key->IsA<APlayerCharacter>()) continue;
		//	if (AEfhorisPlayerController* PlayerController = Pair.Key->GetController<AEfhorisPlayerController>(); IsValid(PlayerController))
		//		if (PlayerController->TargetCounter > 0)
		//			-- PlayerController->TargetCounter;
		//}

		if (const UWorld* World = GetWorld(); IsValid(World)) {
			if (const AEfhorisGameState* GameState = World->GetGameState<AEfhorisGameState>(); IsValid(GameState)) {
				if (AAIManager* Manager = GameState->GetAIManager(); IsValid(Manager)) {
					Manager->RemoveUnit(this);
					Manager->ClearMercenariesThreat(this);
				}
			}
		}
	}
}

FVector AEnemy::StandardMove() const {
	const FVector ActorLocation = GetActorLocation();
	if (!IsValid(Target)) return ActorLocation;

	UInfluenceMapManager* IMapManager = GetWorld()->GetGameState<AEfhorisGameState>()->GetAIManager()->GetIMapManager();
	UInfluenceMap* const Original = IMapManager->GetIMap(ActorLocation, true);
	if (!IsValid(Original)) return ActorLocation;

	UInfluenceMap* const IMap = Original->CopyMap();
	const auto InMapXY = IMap->ToInMapXY(ActorLocation);
	const UInfluenceMap* MoveTemplate = IMapManager->GetTemplate(ETemplateType::Movement);
	const UInfluenceMap* EnemyInfluence = IMapManager->GetTemplate(ETemplateType::Enemy);
	const UInfluenceMap* PlayerTemplate;
	switch (ERole) {
	case ERoleFormation::Melee:
		PlayerTemplate = IMapManager->GetTemplate(ETemplateType::PlayerMelee);
		break;
	case ERoleFormation::Hybrid:
		PlayerTemplate = IMapManager->GetTemplate(ETemplateType::PlayerMelee);
		break;
	case ERoleFormation::Range:
		PlayerTemplate = IMapManager->GetTemplate(ETemplateType::PlayerDistance);
		break;
	case ERoleFormation::None:
		PlayerTemplate = IMapManager->GetTemplate(ETemplateType::PlayerMelee);
		break;
	default:
		PlayerTemplate = IMapManager->GetTemplate(ETemplateType::PlayerMelee);
	}

	//Operations
	IMap->AddAtCenter(EnemyInfluence, InMapXY, 1.0f);
	IMap->AddAtCenter(PlayerTemplate, IMap->ToInMapXY(Target->GetActorLocation()));
	IMap->ApplyAtCenter(MoveTemplate, InMapXY);
	const FIntVector2 HighestInMap = IMap->FindHighest();

	//Movement deemed not worth
	if (FMath::Abs(IMap->GetData(HighestInMap) - IMap->GetData(IMap->ToInMapXY(ActorLocation))) < 0.1f) return ActorLocation;

	//Changes to Original
	//Makes it so others see this unit as half here and half at their goal
	Original->AddAtCenter(EnemyInfluence, InMapXY, 0.5f);
	Original->AddAtCenter(EnemyInfluence, HighestInMap, -0.5f);

	//Return value
	const FVector2D HighestLocation2D = IMap->ToWorldTransform(HighestInMap);
	return FVector(HighestLocation2D.X, HighestLocation2D.Y, 0.0f);
}

void AEnemy::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor); IsValid(Player)) {
		Spot(Player);
	}
}
