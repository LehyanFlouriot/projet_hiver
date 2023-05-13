#include "InteractionComponent.h"

#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "ProjetHiver/Characters/Mercenary.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/Characters/SmithMerchant.h"
#include "ProjetHiver/Contracts/Kevin.h"
#include "ProjetHiver/EfhorisCore/EfhorisHUD.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"
#include "ProjetHiver/GAS/EfhorisAbilitySystemComponent.h"
#include "ProjetHiver/Inventory/InventoryCharacter.h"
#include "ProjetHiver/Inventory/LootChest.h"
#include "ProjetHiver/Inventory/Merchant.h"
#include "ProjetHiver/Items/QuestItem.h"
#include "ProjetHiver/Widgets/GameView.h"
#include "ProjetHiver/Widgets/InteractionPrompt.h"

// Sets default values for this component's properties
UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	//PrimaryComponentTick.TickInterval = 0.5f;
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);
}

void UInteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInteractionComponent, InteractiveTarget);
}

void UInteractionComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if (APlayerCharacter* Owner = Cast<APlayerCharacter>(GetOwner()); ensure(IsValid(Owner))) {
		Owner->OnActorEndOverlap.AddDynamic(this, &UInteractionComponent::OnOwnerEndOverlap);
		Owner->GetOnSetupPlayerInput().AddUObject(this, &UInteractionComponent::OnSetupPlayerInput);
	}
}

void UInteractionComponent::UninitializeComponent()
{
	Super::UninitializeComponent();
	if (APlayerCharacter* Owner = Cast<APlayerCharacter>(GetOwner()); IsValid(Owner)) {
		Owner->OnActorEndOverlap.RemoveDynamic(this, &UInteractionComponent::OnOwnerEndOverlap);
		Owner->GetOnSetupPlayerInput().RemoveAll(this);
	}
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner());
	if (!IsValid(Player) || Player->HasAuthority()) return;
	const AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(Player->GetController());
	if (!IsValid(PlayerController)) return;

	if (const AActor* HitActor = FindInteractiveActorWithRaycast(Player); IsValid(HitActor)) {
		if (!Player->IsInteracting() && !PlayerController->IsInMenuCooldown()) {
			if (const auto HUD = PlayerController->GetHUD<AEfhorisHUD>(); IsValid(HUD))
				if (const auto GameView = HUD->GetGameView(); IsValid(GameView))
					if (const auto InteractionPrompt = GameView->GetInteractPrompt(); IsValid(InteractionPrompt))
						if (const IInteractiveActor* InteractiveActor = Cast<IInteractiveActor>(HitActor))
							InteractionPrompt->SetText(InteractiveActor->GetInteractionText());

		PlayerController->ShowInteractPrompt();
		}
		else {
			PlayerController->HideInteractPrompt();
		}
	}
	else {
		PlayerController->HideInteractPrompt();
	}
}

bool UInteractionComponent::IsInteractible(const APlayerCharacter* InteractionSource, AActor* InteractionTarget) const {
	if (const IInteractiveActor* CastTarget = Cast<IInteractiveActor>(InteractionTarget)) {
		return CastTarget->IsInteractible(InteractionSource);
	}
	return false;
}

void UInteractionComponent::OnOwnerEndOverlap(AActor* OwningActor, AActor* OtherActor) {
	if (const AActor* Owner = GetOwner(); ensure(IsValid(Owner)))
		if (Owner->HasAuthority())
			if (OtherActor->Implements<UInteractiveActor>())
				RemoveInteractiveActorInRange(OtherActor);
}

void UInteractionComponent::OnSetupPlayerInput(UInputComponent* InputComponent) {
	//StartInteract
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
		if (const AActor* Owner = GetOwner(); ensure(IsValid(Owner)))
			if (Owner->IsA<APlayerCharacter>()) {
				const APlayerCharacter* Player = Cast<APlayerCharacter>(Owner);
				EnhancedInputComponent->BindAction(Player->GetInteractAction(), ETriggerEvent::Started, this, &UInteractionComponent::Server_StartInteract);
				EnhancedInputComponent->BindAction(Player->GetInteractAction(), ETriggerEvent::Completed, this, &UInteractionComponent::Server_StopInteract);
			}
}

void UInteractionComponent::RemoveInteractiveActorInRange(AActor* Actor)
{
	//On presume avoir l'autorite
	if (InteractiveTarget == Actor) {
		InteractiveTarget = nullptr;
		if (IInteractiveActor* InteractiveActor = Cast<IInteractiveActor>(Actor); InteractiveActor != nullptr )
			if (AActor* Owner = GetOwner(); IsValid(Owner))
				if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Owner); IsValid(PlayerCharacter))
					InteractiveActor->StopInteract(PlayerCharacter);
	}
}

AActor* UInteractionComponent::FindInteractiveActorWithRaycast(const APlayerCharacter* Source) const noexcept {
	if (const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Source); IsValid(PlayerCharacter)) {
		/*
		 * We want to prevent dead or dying players to interact with things because otherwise,
		 * dying players could resurrect each other.
			*/
		if (!PlayerCharacter->IsAliveTag()) return nullptr;

		const FVector Forward = PlayerCharacter->GetControlRotation().Vector();
		const FVector Start =  PlayerCharacter->GetActorLocation() + PlayerCharacter->GetActorRotation().RotateVector(PlayerCharacter->GetCameraBoom()->SocketOffset);
		const FVector End = (Start + InteractionRange * Forward);
		const UWorld* World = Source->GetWorld();
		FCollisionQueryParams Params(FName(),false,Source);
		FHitResult OutHit;

		//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 5);

		if (World->LineTraceSingleByChannel(OutHit, Start, End, ECC_GameTraceChannel6, Params)) {
			AActor* HitActor = OutHit.GetActor();
			if (IsInteractible(PlayerCharacter, HitActor)){
				return HitActor;
			}
		}
	}
	return nullptr;
}
void UInteractionComponent::Server_StartInteract_Implementation()
{
	Authority_StartInteract();
}

void UInteractionComponent::Server_StopInteract_Implementation()
{
	Authority_StopInteract();
}

void UInteractionComponent::Server_ForceStopInteract_Implementation()
{
	Authority_ForceStopInteract();
}

void UInteractionComponent::Authority_StartInteract() {
	if (AActor* Owner = GetOwner(); IsValid(Owner))
		if (Owner->HasAuthority())
			if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Owner); IsValid(PlayerCharacter)) {
				if (const AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(PlayerCharacter->GetController()); IsValid(PlayerCharacter)) {
					if (!PlayerController->IsInMenuCooldown()) {
						if (!PlayerCharacter->IsInteracting())
						{
							//Start Interaction
							InteractiveTarget = FindInteractiveActorWithRaycast(PlayerCharacter);
							if (IInteractiveActor* Actor = Cast<IInteractiveActor>(InteractiveTarget))
							{
								Actor->StartInteract(PlayerCharacter);
								if (InteractiveTarget->IsA<APlayerCharacter>())
								{
									PlayerCharacter->Server_SetIsInteracting(true);
								}
							}
						}
						else
						{
							//Stop Interaction
							if (IInteractiveActor* Actor = Cast<IInteractiveActor>(InteractiveTarget))
							{
								Actor->StopInteract(PlayerCharacter);
								InteractiveTarget = nullptr;
							}
						}
					}
				}
			}
}

//Used only for player reanimation interactions
void UInteractionComponent::Authority_StopInteract() const
{
	if (AActor* Owner = GetOwner(); IsValid(Owner))
		if (Owner->HasAuthority())
			if (IInteractiveActor* Actor = Cast<IInteractiveActor>(InteractiveTarget))
				if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Owner))
					if (InteractiveTarget->IsA<APlayerCharacter>())
						Actor->StopInteract(PlayerCharacter);
}

void UInteractionComponent::Authority_ForceStopInteract() const
{
	if (AActor* Owner = GetOwner(); IsValid(Owner))
		if (Owner->HasAuthority())
			if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Owner))
				if (IInteractiveActor* Actor = Cast<IInteractiveActor>(InteractiveTarget))
					Actor->StopInteract(PlayerCharacter);
}
