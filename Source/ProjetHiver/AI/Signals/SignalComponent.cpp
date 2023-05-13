// Fill out your copyright notice in the Description page of Project Settings.


#include "SignalComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "ProjetHiver/Characters/Enemy.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/Characters/Mercenary.h"

// Sets default values for this component's properties
USignalComponent::USignalComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void USignalComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USignalComponent, bShowSignals);
}

void USignalComponent::SendSignal(USignal* Signal)
{
	if (!Signal || !Signal->GetEmitter() || !bCanSendSignals) return;
	const auto SpherePos = Signal->GetEmitter()->GetActorLocation();

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel4));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Signal->GetEmitter());

	TArray<AActor*> OutActors;

	if (UKismetSystemLibrary::SphereOverlapActors(GetWorld(), SpherePos, Signal->GetRadius(), ObjectTypes, ABaseCharacter::StaticClass(), ActorsToIgnore, OutActors))
	{
		for (AActor* Actor : OutActors)
		{
			if(const ABaseCharacter* HitActor = Cast<ABaseCharacter>(Actor); IsValid(HitActor))
				HitActor->GetSignalComponent()->ReceiveSignal(Signal);
		}
	}

	if(bShowSignals)
		DrawDebugSphere(GetWorld(), SpherePos, Signal->GetRadius(), 50, FColor::Yellow, false, 5.f);
}

void USignalComponent::ReceiveSignal(USignal* Signal) const
{
	if (!Signal || !bCanReceiveSignals) return;
	switch (Signal->GetSignalType())
	{
	case ESignalType::PlayerAbilityCast: ReceiveSignalPlayerAbilityCast(Signal->GetEmitter()); break;
	case ESignalType::TargetHit: break;
	case ESignalType::PlayerDied: ReceiveSignalPlayerDied(Signal->GetEmitter()); break;
	case ESignalType::NewTargetSensed: ReceiveSignalTargetSensed(Signal->GetEmitter()); break;
	default: break;
	}
}

void USignalComponent::SetShowSignals(bool bShow)
{
	bShowSignals = bShow;
}

// Called when the game starts
void USignalComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USignalComponent::InitializeComponent()
{
	Super::InitializeComponent();
	SetIsReplicated(true);
	if (GetOwner()->IsA<APlayerCharacter>()) {
		bCanSendSignals = true;
		bCanReceiveSignals = false;
	}
	else if (GetOwner()->IsA<AMercenary>())
	{
		bCanSendSignals = false;
		bCanReceiveSignals = true;
	}
	else
	{
		bCanSendSignals = true;
		bCanReceiveSignals = true;
	}
}

void USignalComponent::UninitializeComponent()
{
	Super::UninitializeComponent();
}


// Called every frame
void USignalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void USignalComponent::ReceiveSignalTargetSensed(ABaseCharacter* Emitter) const
{
	if (const ABaseNPC* NPC = Cast<ABaseNPC>(Emitter); IsValid(NPC))
	{
		if(AEnemy* Owner = Cast<AEnemy>(GetOwner()); IsValid(Owner))
		{
			Owner->AddThreat(NPC->GetTarget(), 0.0f, false);
		}
	}
}

void USignalComponent::ReceiveSignalPlayerDied(ABaseCharacter* Emitter) const
{
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Emitter); IsValid(PlayerCharacter))
	{
		if (AMercenary* Owner = Cast<AMercenary>(GetOwner()); IsValid(Owner) && Owner->IsRecruted())
		{
			Owner->AddPlayerToRevive(PlayerCharacter);
		}
	}
}

void USignalComponent::ReceiveSignalPlayerAbilityCast(ABaseCharacter* Emitter) const
{
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Emitter); IsValid(PlayerCharacter))
	{
		if (ABaseNPC* Owner = Cast<ABaseNPC>(GetOwner()); IsValid(Owner))
		{
			UE_LOG(LogTemp, Warning, TEXT("Player is attacking"));
		}
	}
}

