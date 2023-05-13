#include "Mercenary.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy.h"
#include "PlayerCharacter.h"
#include "ProjetHiver/Widgets/FloatingHealthBar.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerState.h"
#include "ProjetHiver/GAS/EfhorisAbilitySystemComponent.h"
#include "ProjetHiver/Characters/AttributeSets/HealthAttributeSet.h"
#include "ProjetHiver/Inventory/PickUpComponent.h"
#include "AttributeSets/PotionAttributeSet.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "ProjetHiver/AI/InfluenceMap/InfluenceMapManager.h"
#include "ProjetHiver/AI/InfluenceMap/TemplateManager.h"
#include "ProjetHiver/AI/InfluenceMap/InfluenceMap.h"
#include "ProjetHiver/AI/AIManager.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/Items/ReplicatedStaticMeshActor.h"

void AMercenary::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMercenary, Master);
}

void AMercenary::ClearUnitReferences() {
	if (IsValid(Master)) {
		Master->RemoveMercenary(this);
	}
	StopCurrentReviveAttempt();
	if (const UWorld* World = GetWorld(); IsValid(World)) {
		if (const AEfhorisGameState* GameState = World->GetGameState<AEfhorisGameState>(); IsValid(GameState)) {
			if (AAIManager* Manager = GameState->GetAIManager(); IsValid(Manager)) {
				Manager->RemoveUnit(this);
				Manager->ClearEnemiesThreat(this);
			}
		}
	}
}

bool AMercenary::IsInteractible(const APlayerCharacter* Player) const {
	return Player->IsAliveTag() && (!HasMaster() || (HasMaster() && Master != Player));
}

void AMercenary::Die()
{
	Super::Die();
	StopCurrentReviveAttempt();

	if (AEfhorisGameMode* GameMode = GetWorld()->GetAuthGameMode<AEfhorisGameMode>(); IsValid(GameMode))
		GameMode->TryLoseGame();
}

void AMercenary::BeginPlay() {
	Super::BeginPlay();

	MoneyCost = MoneyCost * (1 + MoneyScaling * CharacterLevel);

	if (!HasAuthority())
	{
		if (FloatingHealthBar) {
			FloatingHealthBar->SetColor(HealthColor);
		}
	}
	if (HasAuthority() && WeaponMesh)
	{
		AEfhorisGameMode* GameMode = CastChecked<AEfhorisGameMode>(GetWorld()->GetAuthGameMode());

		BackWeapon = GameMode->SpawnActor<AReplicatedStaticMeshActor>(WeaponMesh, FTransform());
		BackWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), "BackHolster");
		SetBackWeaponVisibility(false);
		ActorsToDestroyOnDeath.Add(BackWeapon);

		RightHandWeapon = GameMode->SpawnActor<AReplicatedStaticMeshActor>(WeaponMesh, FTransform());
		RightHandWeapon->AttachToComponent(GetMesh(),FAttachmentTransformRules(EAttachmentRule::SnapToTarget,true), "RightHandSocket");
		SetHandWeaponVisibility(true);
		ActorsToDestroyOnDeath.Add(RightHandWeapon);
	}
}

void AMercenary::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
}

AMercenary::AMercenary(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	PrimaryActorTick.bCanEverTick = true;

	// Create the pick-up component
	PickUpComponent = CreateDefaultSubobject<UPickUpComponent>(TEXT("PickUpComponent"));
	PickUpComponent->SetupAttachment(RootComponent);

	PotionAttributes = CreateDefaultSubobject<UPotionAttributeSet>(TEXT("PotionAttributes"));

	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
}

void AMercenary::StartInteract(APlayerCharacter* Player) {
	if (HasMaster()) {
		SetMaster(Player);
		GetController()->StopMovement();
	}
	else if (AEfhorisGameState* GameState = GetWorld()->GetGameState<AEfhorisGameState>(); IsValid(GameState)) {
		const int NewMoney = GameState->GetMoney() - MoneyCost;
		if (NewMoney >= 0) {
			GameState->SetMoney(NewMoney);
			SetMaster(Player);
		}
	}
}

void AMercenary::StopInteract(APlayerCharacter* Player) {}

const FText AMercenary::GetInteractionText() const
{
	if (HasMaster())
		return InteractionTextChangeOwner;

	return FText::FromString(InteractionTextRecruit.ToString().Replace(*PlaceHolderMoney, *FString::Printf(TEXT("$ %i"), MoneyCost)));
}

float AMercenary::GetPotionCharge() const {
	return PotionAttributes->GetPotionCharge();
}

float AMercenary::GetMaxPotionCharge() const {
	return PotionAttributes->GetMaxPotionCharge();
}

void AMercenary::SetBackWeaponVisibility(const bool bIsHidden) const {
	if (IsValid(BackWeapon))
		BackWeapon->SetActorHiddenInGame(bIsHidden);
}

void AMercenary::SetHandWeaponVisibility(const bool bIsHidden) const {
	if (IsValid(RightHandWeapon))
		RightHandWeapon->SetActorHiddenInGame(bIsHidden);
}

void AMercenary::SetWeaponVisibility(const bool bIsEquipped) const {
	if (bIsEquipped)
	{
		SetBackWeaponVisibility(true);
		SetHandWeaponVisibility(false);
	}
	else
	{
		SetBackWeaponVisibility(false);
		SetHandWeaponVisibility(true);
	}
}

void AMercenary::Regen() {
	ExitCombat();
	ThreatMap.Empty();
	HealthAttributes->SetHealth(GetMaxHealth());
	PotionAttributes->SetPotionCharge(GetMaxPotionCharge());
	AbilitySystemComponent->CancelAllAbilities();
}

bool AMercenary::HasMaster() const noexcept {
	return IsValid(Master);
}

void AMercenary::SetMaster(APlayerCharacter* NewMaster) {
	if (Master) {
		Master->RemoveMercenary(this);
	}
	if (NewMaster) {
		NewMaster->AddMercenary(this);
	}
	Master = NewMaster;
	if (HasMaster()) {
		if (AEfhorisPlayerState* PlayerState_ = Master->GetPlayerState<AEfhorisPlayerState>(); IsValid(PlayerState_))
			SetMasterId(PlayerState_->GetSteamUserId());
	}
	else SetMasterId({});
}

APlayerCharacter* AMercenary::GetMaster() const noexcept {
	return Master;
}

//Changes Master to be the closest alive player.
void AMercenary::FindNewMaster() {
	TArray<APlayerCharacter*> PotentialMasters;
	for (auto EfhorisPlayerState : GetWorld()->GetGameState()->PlayerArray) {
		if (EfhorisPlayerState) {
			if (APlayerCharacter* Player = EfhorisPlayerState->GetPawn<APlayerCharacter>(); IsValid(Player)) {
				if (Player->IsAliveHealth()) {
					PotentialMasters.Add(Player);
				}
			}
		}
	}
	if (!PotentialMasters.IsEmpty()) {
		const FVector MercenaryPosition = GetActorTransform().GetLocation();
		APlayerCharacter* Closest = PotentialMasters[0];
		float ClosestDistance = FVector::DistSquared(Closest->GetActorTransform().GetLocation(), MercenaryPosition);
		for (const auto Player : PotentialMasters) {
			if (const float Distance = FVector::DistSquared(Player->GetActorTransform().GetLocation(), MercenaryPosition); Distance < ClosestDistance) {
				ClosestDistance = Distance;
				Closest = Player;
			}
		}
		SetMaster(Closest);
	}
	else {
		SetMaster(nullptr);
	}
}

FVector AMercenary::LocationAroundMaster() {
	const FVector ActorLocation = GetActorLocation();

	UInfluenceMapManager* IMapManager = GetWorld()->GetGameState<AEfhorisGameState>()->GetAIManager()->GetIMapManager();
	const UInfluenceMap* Original = IMapManager->GetIMap(ActorLocation, true);
	if (!HasMaster() || !IsValid(Original)) return ActorLocation;

	UInfluenceMap* IMap = Original->CopyMap();
	const auto InMapXY = IMap->ToInMapXY(ActorLocation);
	const auto InMapMasterLocation = IMap->ToInMapXY(Master->GetActorLocation());
	const UInfluenceMap* MoveTemplate = IMapManager->GetTemplate(ETemplateType::Movement);
	const UInfluenceMap* AllyUnit = IMapManager->GetTemplate(ETemplateType::AllyUnit);
	const UInfluenceMap* MasterInfluence = IMapManager->GetTemplate(ETemplateType::PlayerMelee);

	//Operations
	IMap->AddAtCenter(AllyUnit, InMapMasterLocation, 1.0f);
	IMap->AddAtCenter(MasterInfluence, InMapMasterLocation, 2.0f);
	IMap->ApplyAtCenter(MoveTemplate, InMapXY);
	if (bShowIMap)
		IMap->Draw(GetWorld(), 0.3);
	const FIntVector2 HighestInMap = IMap->FindHighest();

	//Return value
	const FVector2D HighestLocation2D = IMap->ToWorldTransform(HighestInMap);
	return FVector(HighestLocation2D.X, HighestLocation2D.Y, 0.0f);
}

void AMercenary::Authority_Teleport(const FVector& Position) {
	SetActorLocation(Position);
	ClearAllRevive();
	ThreatMap.Empty();
	SetTarget(nullptr);
	AbilitySystemComponent->CancelAllAbilities();
}

bool AMercenary::IsRecruted() const noexcept {
	return Master != nullptr;
}

void AMercenary::TryCancelCombo() {
	if (HasTarget() && !Target->IsAliveHealth())
		CancelCombo();
}

void AMercenary::CancelCombo() {
	FGameplayTagContainer AttackTags;
	AttackTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.Attack")));
	AttackTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.RangeAttack")));
	AbilitySystemComponent->CancelAbilities(&AttackTags);
}

FVector AMercenary::FindHealingLocation() {
	const FVector ActorLocation = GetActorLocation();

	UInfluenceMapManager* IMapManager = GetWorld()->GetGameState<AEfhorisGameState>()->GetAIManager()->GetIMapManager();
	const UInfluenceMap* Original = IMapManager->GetIMap(ActorLocation, true);
	if (!IsValid(Original)) return ActorLocation;

	UInfluenceMap* IMap = Original->CopyMap();
	const auto InMapXY = IMap->ToInMapXY(ActorLocation);
	const UInfluenceMap* MoveTemplate = IMapManager->GetTemplate(ETemplateType::Movement);
	const UInfluenceMap* SelfInfluence = IMapManager->GetTemplate(ETemplateType::AllyUnit);

	//Operations
	IMap->AddAtCenter(SelfInfluence, InMapXY, 1.0f);
	IMap->ApplyAtCenter(MoveTemplate, InMapXY);
	if (bShowIMap)
		IMap->Draw(GetWorld(), 5);
	const FIntVector2 HighestInMap = IMap->FindHighest();

	//Return value
	const FVector2D HighestLocation2D = IMap->ToWorldTransform(HighestInMap);
	return FVector(HighestLocation2D.X, HighestLocation2D.Y, 0.0f);
}

//Threshold is health percentage
bool AMercenary::ShouldHeal(const float Threshold) const {
	return GetPotionCharge() > 0 && GetHealth() / GetMaxHealth() < Threshold/100.0f;
}

void AMercenary::StopCurrentReviveAttempt() {
	if (IsValid(PlayerBeingRevived))
		PlayerBeingRevived->StopMercenaryReviveMe(this);
}

void AMercenary::DeletePlayerToRevive(APlayerCharacter* PlayerToRevive) noexcept {
	PlayersToRevive.RemoveSingle(PlayerToRevive);
	if (PlayerBeingRevived == PlayerToRevive)
		StopCurrentReviveAttempt();
}

void AMercenary::ClearAllRevive() {
	StopCurrentReviveAttempt();
	PlayersToRevive.Empty();
}