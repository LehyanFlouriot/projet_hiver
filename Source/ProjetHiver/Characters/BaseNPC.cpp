#include "BaseNPC.h"

#include "AIController.h"
#include "Mercenary.h"
#include "Enemy.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Net/UnrealNetwork.h"
#include "Perception/PawnSensingComponent.h"
#include "ProjetHiver/AI/AIManager.h"
#include "ProjetHiver/AI/Signals/Signal.h"
#include "ProjetHiver/AI/Signals/SignalComponent.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/Characters/AttributeSets/HealthAttributeSet.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerState.h"
#include "ProjetHiver/GAS/EfhorisAbilitySystemComponent.h"
#include "ProjetHiver/Widgets/FloatingHealthBar.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "ProjetHiver/LootComponent/LootComponent.h"

void ABaseNPC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseNPC, bIsInCombat);
}

void ABaseNPC::SetHealthBarVisibility(const bool bIsVisible) const {
	if (FloatingHealthBar)
	{
		if (bIsVisible) FloatingHealthBar->SetVisibility(ESlateVisibility::Visible);
		else FloatingHealthBar->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ABaseNPC::ClearUnitReferences() {}

ABaseNPC::ABaseNPC(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	FloatingHealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("FloatingHealthBarComponent"));
	FloatingHealthBarComponent->SetupAttachment(RootComponent);

	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
}

void ABaseNPC::BeginPlay()
{
	Super::BeginPlay();

	InitializeAbilityActorInfo();
	InitializeNPCAbilities();
	InitializeAttributes();

	if (!HasAuthority() && FloatingHealthBarClass)
	{
		FloatingHealthBar = CreateWidget<UFloatingHealthBar>(GetWorld()->GetFirstPlayerController(), FloatingHealthBarClass);
		if (FloatingHealthBar && FloatingHealthBarComponent)
		{
			FloatingHealthBarComponent->SetWidget(FloatingHealthBar);
			FloatingHealthBarComponent->CastShadow = false;
			FloatingHealthBar->SetHealthPercentage(GetHealth() / GetMaxHealth());
		}
	}

	HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(HealthAttributes->GetHealthAttribute()).AddUObject(this, &ABaseNPC::HealthChanged);
	
	//Sensing
	SensingComponent = Cast<UPawnSensingComponent>(GetComponentByClass(UPawnSensingComponent::StaticClass()));
	if (SensingComponent != nullptr) {
		SensingComponent->SightRadius = VisionRange;
		SensingComponent->SetPeripheralVisionAngle(VisionAngle);
		SensingComponent->OnSeePawn.AddDynamic(this, &ABaseNPC::OnSeePawn);
	}

	InitialLocation = GetActorLocation();
	InitialLocation.Z = 0;

	//AIManager
	if (AAIManager* Manager = GetWorld()->GetGameState<AEfhorisGameState>()->GetAIManager(); IsValid(Manager)) {
		Manager->AddUnit(this);
	}

	Landed({});
}

void ABaseNPC::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority()) {
		SetActorRotation(FRotator{ 0, GetActorRotation().Yaw,  0 });
	}
}

void ABaseNPC::K2_DestroyActor() {
	ClearUnitReferences();
	for (AActor* Actor : ActorsToDestroyOnDeath)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	Super::K2_DestroyActor();
}

void ABaseNPC::HealthChanged(const FOnAttributeChangeData& Data)
{
	const float Health = Data.NewValue;

	if (FloatingHealthBar) FloatingHealthBar->SetHealthPercentage(Health / GetMaxHealth());
}

#pragma region Threat

void ABaseNPC::OnSeePawn(APawn* OtherPawn) {
	if (ABaseCharacter* Character = Cast<ABaseCharacter>(OtherPawn)) {
		if (Character->IsAliveHealth() && Character->GetFactionTag() != GetFactionTag() && !ThreatMap.Contains(Character)) {
			//Enemies can't see non recruited mercenaries
			if (const AMercenary* Mercenary = Cast<AMercenary>(Character); IsValid(Mercenary) && !Mercenary->HasMaster()) return;
			Spot(Character);
		}
	}
}

void ABaseNPC::Spot(ABaseCharacter* Target_) {
	if (!ThreatMap.Contains(Target_)) {
		AddThreat(Target_, 0.0f);
	}
}

TPair<ABaseCharacter*, int> ABaseNPC::MaxThreat() {
	auto Max = TPair<ABaseCharacter*, int>(nullptr, -1);
	for(const TPair<ABaseCharacter*, int>& Pair : ThreatMap) {
		if (Pair.Value > Max.Value) {
			Max = Pair;
		}
	}
	return Max;
}

void ABaseNPC::EvaluateNewTarget() {
	if (IsValid(Target)) {
		if (ThreatMap.Num() <= 1) return;
		const int CurrentThreat = *ThreatMap.Find(Target);
		const int MeleeThreshold = 1.1f * CurrentThreat;
		const int DistanceThreshold = 1.3f * CurrentThreat;
		const auto MostThreat = MaxThreat();
		const int Threshold = MostThreat.Key->GetDistanceTo(this) < 500.0f ? MeleeThreshold : DistanceThreshold;
		if (MostThreat.Value > Threshold) {
			SetTarget(MostThreat.Key);
		}
	}
	else {
		if (ThreatMap.Num()>=1) {
			SetTarget(ThreatMap.begin()->Key);
		}
	}
}

void ABaseNPC::SetTarget(ABaseCharacter* Target_) {
	Target = Target_;
	if (APlayerCharacter* PlayerTarget = Cast<APlayerCharacter>(Target); IsValid(PlayerTarget)) {
		if (AEfhorisPlayerState* PlayerState_ = PlayerTarget->GetPlayerState<AEfhorisPlayerState>(); IsValid(PlayerState_))
			SetTargetId(PlayerState_->GetSteamUserId());
	} else if (!HasTarget()) SetTargetId({});

	AAIController* AIController = Cast<AAIController>(GetController());
	if (IsValid(AIController)) {
		if (IsValid(Target_)) {
			AIController->SetFocus(Target);
		}
		else {
			AIController->ClearFocus(2);
		}
	}
}

void ABaseNPC::AddThreat(ABaseCharacter* Target_, const int& ThreatAmount, const bool bPropagate) {
	bool bShouldSendSignal = false;
	if (ThreatMap.IsEmpty()) {
		EnterCombat();
	}
	if (const int* PThreat = ThreatMap.Find(Target_)) {
		ThreatMap.Add(Target_, *PThreat + ThreatAmount);
	}
	else {
		ThreatMap.Add(Target_,  ThreatAmount);
		if (AEfhorisPlayerController* PlayerController = Target_->GetController<AEfhorisPlayerController>(); IsValid(PlayerController)) {
			if(HasAuthority())
			{
				if(!PlayerController->MusicTargets.Contains(this))
					PlayerController->MusicTargets.Add(this);
			}
			PlayerController->Client_ShowFloatingHealthBar(this, true);
		}
		bShouldSendSignal = true;
	}
	EvaluateNewTarget();
	if (bShouldSendSignal && bPropagate && IsA<AEnemy>())
	{
		USignal* Signal = NewObject<USignal>();
		Signal->Init(ESignalType::NewTargetSensed, 1000.f, this);
		SignalComponent->SendSignal(Signal);
	}
}

void ABaseNPC::ClearThreat(ABaseCharacter* Target_) {
	ThreatMap.Remove(Target_);
	if (ThreatMap.IsEmpty() && bIsInCombat) {
		ExitCombat();
	}

	if (AEfhorisPlayerController* PlayerController = Target_->GetController<AEfhorisPlayerController>(); IsValid(PlayerController)) {
		PlayerController->Client_ShowFloatingHealthBar(this, false);
	}
	SetTarget(nullptr);
	EvaluateNewTarget();
	
}

void ABaseNPC::Taunt(ABaseCharacter* Source, const float TauntStrength)
{
	for (auto& Threat : ThreatMap)
		Threat.Value = 0;

	if (Target != Source)
	{
		GetController()->StopMovement();
	}
	AddThreat(Source, TauntStrength);
}

TMap<ABaseCharacter*, int>& ABaseNPC::GetThreatMap() {
	return ThreatMap;
}

void ABaseNPC::DrawDebugThreat(const float& DeltaSeconds) {
	const FVector SelfLocation = GetTransform().GetLocation();
	for (const TPair<ABaseCharacter*, int>& Pair : GetThreatMap()) {
		const FColor Color = Pair.Key == GetTarget() ? FColor::Red : FColor::Green;
		const FVector TargetLocation = Pair.Key->GetTransform().GetLocation();
		const FVector StringLocation = (SelfLocation + TargetLocation) / 2;
		DrawDebugLine(GetWorld(), SelfLocation, TargetLocation, Color, false,DeltaSeconds);
		DrawDebugString(GetWorld(), StringLocation, FString::FromInt(Pair.Value), nullptr, Color, DeltaSeconds);
	}
}

#pragma endregion

void ABaseNPC::Die()
{
	Super::Die();
	if(HasAuthority())
	{
		if(auto GameState = GetWorld()->GetGameState(); IsValid(GameState))
		{
			for (auto _PlayerState : GameState->PlayerArray)
			{
				if(auto PlayerController = Cast<AEfhorisPlayerController>(_PlayerState->GetPlayerController()); IsValid(PlayerController))
				{
					if(PlayerController->MusicTargets.Contains(this))
					{
						PlayerController->MusicTargets.Remove(this);
					}
				}
			}
		}
	}

	ClearUnitReferences();
	Multicast_SetCollision(ECollisionEnabled::NoCollision);
	GetController()->StopMovement();
	APawn::DetachFromControllerPendingDestroy();
	FTimerHandle UnusedHandle;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([&]
	{
		for (AActor* Actor : ActorsToDestroyOnDeath)
		{
			if(IsValid(Actor))
			{
				Actor->Destroy();
			}
		}
		AActor::Destroy();
	});
	GetWorldTimerManager().SetTimer(UnusedHandle, TimerDelegate, DieTimer, false);
}

void ABaseNPC::MoveTo(const FVector& Location, const float AcceptanceRadius) const {
	const float SmallerAcceptance = FMath::Max(0.0f,0.9 * AcceptanceRadius- 30.0f);
	AAIController* AIController = Cast<AAIController>(GetController());
	AIController->MoveToLocation(Location, SmallerAcceptance,false);
}

void ABaseNPC::MoveToCombatRange() const {
	MoveTo(Target->GetActorLocation(),CombatRange);
}

void ABaseNPC::ChooseCombo()
{
	float WeightSum = 0;
	constexpr float RebalanceFactor = 0.75f;
	constexpr float InverseRebalanceFactor = 1/ RebalanceFactor;

	for(const FAIAbility& CombatAbility : CombatCombos)
	{
		WeightSum += CombatAbility.Weight;
	}
	float RandomWeight = FMath::RandRange(0.0f, WeightSum);
	for (int i = 0; i < CombatCombos.Num(); ++i) {
		RandomWeight -= CombatCombos[i].Weight;
		if (RandomWeight <= 0) {
			CombatCombos[i].Weight *= RebalanceFactor;
			for(int j = 0; j < CombatCombos.Num(); ++j)
			{
				if(i != j)
				{
					CombatCombos[j].Weight *= InverseRebalanceFactor;
				}
			}
			ChosenCombo = CombatCombos[i];
			break;
		}
	}
}

void ABaseNPC::InitializeNPCAbilities()
{
	InitializeAbilities();
	if (HasAuthority() && IsValid(AbilitySystemComponent))
	{
		for (FAIAbility& Combo : CombatCombos)
			if (Combo.Combo)
				AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Combo.Combo, 1, static_cast<int32>(Combo.Combo.GetDefaultObject()->GetAbilityInputID()), this));
	}
}
