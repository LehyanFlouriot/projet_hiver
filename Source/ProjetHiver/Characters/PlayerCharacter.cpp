// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Enemy.h"
#include "Mercenary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "AttributeSets/PotionAttributeSet.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/CompositeDataTable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "ProjetHiver/AI/AIManager.h"
#include "ProjetHiver/AI/PlayersStatsMeasuringComponent.h"
#include "ProjetHiver/Characters/AttributeSets/HealthAttributeSet.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameInstance.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"
#include "ProjetHiver/GAS/EfhorisAbilitySystemComponent.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "ProjetHiver/Interaction/InteractionComponent.h"
#include "ProjetHiver/Inventory/Merchant.h"
#include "ProjetHiver/Inventory/PickUpComponent.h"
#include "ProjetHiver/Items/ReplicatedStaticMeshActor.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/SpectatorPawn.h"
#include "Kismet/GameplayStatics.h"
#include "ProjetHiver/AI/Signals/Signal.h"
#include "ProjetHiver/AI/Signals/SignalComponent.h"
#include "ProjetHiver/EfhorisCore/EfhorisHUD.h"
#include "ProjetHiver/Widgets/GameView.h"
#include "ProjetHiver/Widgets/SpellSlot.h"

//////////////////////////////////////////////////////////////////////////
// APlayerCharacter

#define BIND_HANDLERS(ActionName) \
	EnhancedInputComponent->BindAction(ActionName, ETriggerEvent::Triggered, this, &APlayerCharacter::Handle##ActionName##Pressed); \
	EnhancedInputComponent->BindAction(ActionName, ETriggerEvent::Completed, this, &APlayerCharacter::Handle##ActionName##Released);


APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// Create the pick-up component
	PickUpComponent = CreateDefaultSubobject<UPickUpComponent>(TEXT("PickUpComponent"));
	PickUpComponent->SetupAttachment(RootComponent);

	// Create the interaction component
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
	InteractionComponent->SetupAttachment(RootComponent);
	
	// Create the statistics measuring component
	PlayersStatsMeasuringComponent = CreateDefaultSubobject<UPlayersStatsMeasuringComponent>(TEXT("PlayersStatsMeasuringComponent"));
	PlayersStatsMeasuringComponent->SetupAttachment(RootComponent);

	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	PotionAttributes = CreateDefaultSubobject<UPotionAttributeSet>(TEXT("PotionAttributes"));

	FloatingNameComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("FloatingNameComponent"));
	FloatingNameComponent->SetupAttachment(RootComponent);
}

void APlayerCharacter::OnRep_RefreshSpellsPanelWidget()
{
	auto LocalController = GetWorld()->GetFirstPlayerController<AEfhorisPlayerController>();
	auto LocalCharacter = LocalController->GetPawn<APlayerCharacter>();

	if(LocalCharacter == this)
	{
		if (const AEfhorisHUD* HUD = Cast<AEfhorisHUD>(LocalController->GetHUD()); IsValid(HUD))
			if (UGameView* GameView = HUD->GetGameView(); IsValid(GameView))
				GameView->RefreshSpellsPanel();
	}
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (IsValid(AbilitySystemComponent))
	{
		//Server GAS init
		InitializeAbilityActorInfo();
		InitializeAbilities();
		InitializeAttributes();
	}
}

FPlayerOptions& APlayerCharacter::GetPlayerOptions() noexcept {
	return Options;
}

void APlayerCharacter::Server_AddInteraction_Implementation() {
	NbInteraction++;
	if (NbInteraction == 1) {
		Server_SetIsInteracting(true);
	}
}

void APlayerCharacter::Server_RemoveInteraction_Implementation() {
	NbInteraction = FMath::Max(0, NbInteraction - 1);
	if (NbInteraction == 0)
	{
		Server_SetAllInteractionBooleans(false,false,false);
	}
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APlayerCharacter, WeaponKitRow);

	DOREPLIFETIME(APlayerCharacter, ArmorKitInfo);
	DOREPLIFETIME(APlayerCharacter, WeaponKitInfo);
	DOREPLIFETIME(APlayerCharacter, GlovesInfo);
	DOREPLIFETIME(APlayerCharacter, BeltInfo);
	DOREPLIFETIME(APlayerCharacter, RingInfo);
	DOREPLIFETIME(APlayerCharacter, NecklaceInfo);
	DOREPLIFETIME(APlayerCharacter, EarringsInfo);

	DOREPLIFETIME(APlayerCharacter, bHasArmorKit);
	DOREPLIFETIME(APlayerCharacter, bHasWeaponKit);
	DOREPLIFETIME(APlayerCharacter, bHasGloves);
	DOREPLIFETIME(APlayerCharacter, bHasBelt);
	DOREPLIFETIME(APlayerCharacter, bHasRing);
	DOREPLIFETIME(APlayerCharacter, bHasNecklace);
	DOREPLIFETIME(APlayerCharacter, bHasEarrings);

	DOREPLIFETIME(APlayerCharacter, NumberOfBasicAttacks);
	DOREPLIFETIME(APlayerCharacter, bIsInteracting);
	DOREPLIFETIME(APlayerCharacter, bIsTrading);
	DOREPLIFETIME(APlayerCharacter, bIsSmithing);
	DOREPLIFETIME(APlayerCharacter, SmithItem);
	DOREPLIFETIME(APlayerCharacter, bHasSmithItem);
}

void APlayerCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	bCanCancelAbility = true;

	//Add Input Mapping Context
	if (const APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (HasAuthority()) {
		UnequipArmorKit(true);
		UnequipWeaponKit(false);
	}
	else {
		ResetArmorMeshes();
	}
}

void APlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		if (IsValid(RightHandWeaponActor))
			RightHandWeaponActor->Destroy();

		if (IsValid(LeftHandWeaponActor))
			LeftHandWeaponActor->Destroy();
	}

	Super::EndPlay(EndPlayReason);
}

//////////////////////////////////////////////////////////////////////////
// Input

void APlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	OnSetupPlayerInput.Broadcast(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &APlayerCharacter::ResetLastInputVector);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

		BIND_HANDLERS(JumpAction);
		BIND_HANDLERS(DodgeAction);
		BIND_HANDLERS(PotionAction);
		BIND_HANDLERS(AttackComboAction);
		BIND_HANDLERS(RightClickAction);
		BIND_HANDLERS(Skill1Action);
		BIND_HANDLERS(Skill2Action);

		//Call Inventory
		EnhancedInputComponent->BindAction(CallInventoryAction, ETriggerEvent::Started, this, &APlayerCharacter::CallInventory);

		//Confirm & Cancel
		EnhancedInputComponent->BindAction(ConfirmAction, ETriggerEvent::Triggered, this, &APlayerCharacter::HandleConfirmActionPressed);
		EnhancedInputComponent->BindAction(CancelAction, ETriggerEvent::Triggered, this, &APlayerCharacter::HandleCancelActionPressed);
	}

	if (IsValid(AbilitySystemComponent) && IsValid(InputComponent))
	{
		const FTopLevelAssetPath AbilityEnumAssetPath = FTopLevelAssetPath(FName("/Script/ProjetHiver"), FName("EGASAbilityInputID"));
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds(FString("ConfirmTarget"),
			FString("CancelTarget"), AbilityEnumAssetPath, static_cast<int32>(EGASAbilityInputID::Confirm), static_cast<int32>(EGASAbilityInputID::Cancel)));
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	if (bIsInteracting || !IsAliveTag()) return;

	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);

		LastInputVector = MovementVector;
		ServerRPCLastInputVector(LastInputVector);
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	if (bIsInteracting || !IsAliveTag()) return;

	// input is a Vector2D
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APlayerCharacter::CallInventory()
{
	if (IsAliveTag()) {
		auto PC = CastChecked<AEfhorisPlayerController>(GetLocalViewingPlayerController());
		PC->Server_CallInventoryRequest();
		PC->PlayCallInventorySound();
	}
}

void APlayerCharacter::HandleAttackComboActionPressed()
{
	if (bIsInteracting) return;

	if (bCanProgressCombo)
	{
		USignal* Signal = NewObject<USignal>();
		Signal->Init(ESignalType::PlayerAbilityCast, 300.f, this);
		SignalComponent->SendSignal(Signal);

		ProgressCombo();
		bEndMeleeNormally = false;

		SendLocalInputToASC(true, static_cast<int32>(EGASAbilityInputID::AttackComboFirstAction) + ComboProgress);
	}
	else
		SendLocalInputToASC(true, static_cast<int32>(EGASAbilityInputID::AttackComboFirstAction) + ComboProgress);
}

void APlayerCharacter::HandleAttackComboActionReleased()
{
	SendLocalInputToASC(false, static_cast<int32>(EGASAbilityInputID::AttackComboFirstAction) + ComboProgress);
}

void APlayerCharacter::HandleConfirmActionPressed()
{
	AbilitySystemComponent->LocalInputConfirm();
}

void APlayerCharacter::HandleCancelActionPressed()
{
	AbilitySystemComponent->LocalInputCancel();
}

void APlayerCharacter::SendLocalInputToASC(const bool bIsPressed, const EGASAbilityInputID AbilityInputID) const
{
	SendLocalInputToASC(bIsPressed, static_cast<int32>(AbilityInputID));
}

void APlayerCharacter::SendLocalInputToASC(const bool bIsPressed, const int32 AbilityInputID) const
{
	if (bIsPressed)
		AbilitySystemComponent->AbilityLocalInputPressed(AbilityInputID);
	else
		AbilitySystemComponent->AbilityLocalInputReleased(AbilityInputID);
}

void APlayerCharacter::ResetLastInputVector()
{
	LastInputVector = FVector2D{ 0.f, 0.f };
	ServerRPCLastInputVector(LastInputVector);
}

void APlayerCharacter::ProgressCombo()
{
	bCanProgressCombo = false;
	bEndMeleeNormally = true;
	bCanCancelAbility = true;

	ComboProgress++;
	if (ComboProgress == NumberOfBasicAttacks)
		ComboProgress = 0;

	MeleeHitTargets.Empty();

	FGameplayTagContainer AttackTags;
	AttackTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.Attack")));
	AttackTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.RangeAttack")));
	AbilitySystemComponent->CancelAbilities(&AttackTags);
}

void APlayerCharacter::InstantDeath_Implementation() {
	const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName("Character.Status.IsDead"));
	const FGameplayTag DyingTag = FGameplayTag::RequestGameplayTag(FName("Character.Status.IsDying"));
	const FGameplayTagContainer Container = FGameplayTagContainer(DyingTag);

	if (AbilitySystemComponent->HasMatchingGameplayTag(DyingTag))
	{
		AbilitySystemComponent->CancelAbilities(&Container);
		
	}
	AbilitySystemComponent->AddReplicatedLooseGameplayTag(DeadTag);
	AbilitySystemComponent->AddLooseGameplayTag(DeadTag);

	AEfhorisGameMode* GameMode = GetWorld()->GetAuthGameMode<AEfhorisGameMode>();
	GameMode->AddPawnToDeleteOnLevelReset(this);

	ASpectatorPawn* Spectator = GameMode->SpawnActor<ASpectatorPawn>(GameMode->SpectatorClass, GetFollowCamera()->GetComponentTransform());
	GetController()->Possess(Spectator);

	GameMode->TryLoseGame();
}

void APlayerCharacter::ResetCombo()
{
	ComboProgress = 0;
	bCanCancelAbility = true;
}

bool APlayerCharacter::GetEndMeleeNormally() const
{
	return bEndMeleeNormally;
}

void APlayerCharacter::ServerRPCLastInputVector_Implementation(const FVector2D InputVector)
{
	LastInputVector = InputVector;
}

void APlayerCharacter::SetComboGivenAbilitiesFromBuff(TArray<TSubclassOf<UEfhorisGameplayAbility>> _BuffGivenAbilities)
{
	for (const auto AbilitySpecHandle : WeaponKitBasicAttacksGivenAbilities)
		AbilitySystemComponent->ClearAbility(AbilitySpecHandle);
	WeaponKitBasicAttacksGivenAbilities.Empty();

	NumberOfBasicAttacks = 0;
	for (const auto BuffGivenAbility : _BuffGivenAbilities)
	{
		WeaponKitBasicAttacksGivenAbilities.Add(AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(BuffGivenAbility, WeaponKitInfo.Level, static_cast<int32>(EGASAbilityInputID::AttackComboFirstAction) + NumberOfBasicAttacks, this)));
		++NumberOfBasicAttacks;
	}
}

void APlayerCharacter::RemoveComboGivenAbilitiesFromBuff()
{
	for (const auto AbilitySpecHandle : WeaponKitBasicAttacksGivenAbilities)
		AbilitySystemComponent->ClearAbility(AbilitySpecHandle);
	WeaponKitBasicAttacksGivenAbilities.Empty();

	WeaponKitRow = *CastChecked<UEfhorisGameInstance>(GetWorld()->GetGameInstance())->GetWeaponKitsDataTable()->FindRow<FWeaponKitRow>(WeaponKitInfo.RowName, {});
	NumberOfBasicAttacks = WeaponKitRow.BaseAttackCombo.Num();

	int32 InputId = static_cast<int32>(EGASAbilityInputID::AttackComboFirstAction);

	for (TSubclassOf<UEfhorisGameplayAbility> Attack : WeaponKitRow.BaseAttackCombo)
	{
		if (IsValid(Attack))
		{
			WeaponKitBasicAttacksGivenAbilities.Add(AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Attack, WeaponKitInfo.Level, InputId, this)));
			InputId++;
		}
	}
}

FVector2D APlayerCharacter::GetLastInputVector() const noexcept
{
	return LastInputVector;
}

void APlayerCharacter::EquipArmorKit(const FItemInfo& ArmorKit)
{
	if (HasAuthority() && ArmorKit.ItemType == EItemType::Armor)
	{
		if (bHasArmorKit)
			UnequipArmorKit();

		bHasArmorKit = true;
		ArmorKitInfo = ArmorKit;
		ArmorGivenStats = AddEquipmentStats(ArmorKitInfo.ItemStats);

		Multicast_SpawnArmorMeshes(ArmorKitInfo.RowName);
	}
}

void APlayerCharacter::UnequipArmorKit(const bool bIsFirstInit)
{
	if (HasAuthority() || bIsFirstInit)
	{
		bHasArmorKit = false;
		ArmorKitInfo = {};
		ArmorKitInfo.ItemType = EItemType::Armor;
		AbilitySystemComponent->RemoveActiveGameplayEffect(ArmorGivenStats);

		Multicast_RemoveArmorMeshes();
	}
}

void APlayerCharacter::SpawnArmorMeshes(FName RowName)
{
	if (const FArmorKitRow* ArmorKitRow = Cast<UEfhorisGameInstance>(GetWorld()->GetGameInstance())->GetArmorKitsDataTable()->FindRow<FArmorKitRow>(ArmorKitInfo.RowName, {}); ArmorKitRow && IsValid(ArmorKitRow->RootMesh))
	{
		for (USkeletalMeshComponent* MeshComponent : ArmorMeshes)
		{
			MeshComponent->DestroyComponent();
		}
		ArmorMeshes.Empty();

		bUseControlRig = ArmorKitRow->bUseControlRig;

		GetMesh()->SetSkeletalMesh(ArmorKitRow->RootMesh);

		for (USkeletalMesh* SkeletalMesh : ArmorKitRow->ArmorMeshes)
		{
			if (IsValid(SkeletalMesh))
			{
				USkeletalMeshComponent* MeshComponent = NewObject<USkeletalMeshComponent>(this, USkeletalMeshComponent::StaticClass(), NAME_None, RF_Transient);
				MeshComponent->SetupAttachment(GetMesh());
				MeshComponent->RegisterComponent();

				MeshComponent->SetSkeletalMesh(SkeletalMesh);
				MeshComponent->SetLeaderPoseComponent(GetMesh());
				MeshComponent->SetReceivesDecals(false);
				ArmorMeshes.Add(MeshComponent);
			}
		}
	}
}

void APlayerCharacter::RemoveArmorMeshes()
{
	if (IsValid(DefaultRootMesh))
	{
		for (USkeletalMeshComponent* MeshComponent : ArmorMeshes)
		{
			MeshComponent->DestroyComponent();
		}
		ArmorMeshes.Empty();

		bUseControlRig = true;

		GetMesh()->SetSkeletalMesh(DefaultRootMesh);

		for (USkeletalMesh* SkeletalMesh : DefaultSubMeshes)
		{
			if (IsValid(SkeletalMesh))
			{
				USkeletalMeshComponent* MeshComponent = NewObject<USkeletalMeshComponent>(this, USkeletalMeshComponent::StaticClass(), NAME_None, RF_Transient);
				MeshComponent->SetupAttachment(GetMesh());
				MeshComponent->RegisterComponent();

				MeshComponent->SetSkeletalMesh(SkeletalMesh);
				MeshComponent->SetLeaderPoseComponent(GetMesh());
				MeshComponent->SetReceivesDecals(false);
				ArmorMeshes.Add(MeshComponent);
			}
		}
	}
}

void APlayerCharacter::ResetArmorMeshes()
{
	if (ArmorKitInfo.RowName == FName("None"))
		RemoveArmorMeshes();
	else
		SpawnArmorMeshes(ArmorKitInfo.RowName);
}

void APlayerCharacter::Multicast_SpawnArmorMeshes_Implementation(FName RowName)
{
	SpawnArmorMeshes(RowName);
}

void APlayerCharacter::Multicast_RemoveArmorMeshes_Implementation()
{
	RemoveArmorMeshes();
}

void APlayerCharacter::EquipWeaponKit(const FItemInfo& WeaponKit)
{
	if (HasAuthority() && WeaponKit.ItemType == EItemType::Weapon)
	{
		if (bHasWeaponKit)
			UnequipWeaponKit(true);

		bHasWeaponKit = true;
		WeaponKitInfo = WeaponKit;
		WeaponGivenStats = AddEquipmentStats(WeaponKitInfo.ItemStats);

		AEfhorisGameMode* GameMode = CastChecked<AEfhorisGameMode>(GetWorld()->GetAuthGameMode());
		WeaponKitRow = *CastChecked<UEfhorisGameInstance>(GetWorld()->GetGameInstance())->GetWeaponKitsDataTable()->FindRow<FWeaponKitRow>(WeaponKitInfo.RowName, {});
		NumberOfBasicAttacks = WeaponKitRow.BaseAttackCombo.Num();

		if (IsValid(WeaponKitRow.RightHandActor))
		{
			const FName SocketName = FName("RightHandSocket");
			const auto Transform = GetMesh()->GetSocketTransform(SocketName);

			RightHandWeaponActor = GameMode->SpawnActor<AReplicatedStaticMeshActor>(WeaponKitRow.RightHandActor.Get(), Transform);
			RightHandWeaponActor->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), SocketName);
			RightHandWeaponActor->GetStaticMeshComponent()->SetVisibility(true);
		}

		if (IsValid(WeaponKitRow.LeftHandActor))
		{
			const FName SocketName = FName("LeftHandSocket");
			const auto Transform = GetMesh()->GetSocketTransform(SocketName);

			LeftHandWeaponActor = GameMode->SpawnActor<AReplicatedStaticMeshActor>(WeaponKitRow.LeftHandActor.Get(), Transform);
			LeftHandWeaponActor->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), SocketName);
			LeftHandWeaponActor->GetStaticMeshComponent()->SetVisibility(true);
		}

		if (IsValid(DefaultDodgeAbility))
			AbilitySystemComponent->ClearAllAbilitiesWithInputID(static_cast<int32>(DefaultDodgeAbility.GetDefaultObject()->GetAbilityInputID()));

		if (IsValid(WeaponKitRow.RightClickAbility))
			WeaponKitGivenAbilities.Add(AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(WeaponKitRow.RightClickAbility, WeaponKitInfo.Level, static_cast<int32>(EGASAbilityInputID::RightClickAction), this)));

		if (IsValid(WeaponKitInfo.Ability1))
			WeaponKitGivenAbilities.Add(AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(WeaponKitInfo.Ability1, WeaponKitInfo.Level, static_cast<int32>(EGASAbilityInputID::Skill1Action), this)));

		if (IsValid(WeaponKitInfo.Ability2))
			WeaponKitGivenAbilities.Add(AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(WeaponKitInfo.Ability2, WeaponKitInfo.Level, static_cast<int32>(EGASAbilityInputID::Skill2Action), this)));

		if (IsValid(WeaponKitRow.DodgeAbility))
			WeaponKitGivenAbilities.Add(AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(WeaponKitRow.DodgeAbility, WeaponKitInfo.Level, static_cast<int32>(EGASAbilityInputID::DodgeAction), this)));

		int32 InputId = static_cast<int32>(EGASAbilityInputID::AttackComboFirstAction);

		for (TSubclassOf<UEfhorisGameplayAbility> Attack : WeaponKitRow.BaseAttackCombo)
		{
			if (IsValid(Attack))
			{
				WeaponKitBasicAttacksGivenAbilities.Add(AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Attack, WeaponKitInfo.Level, InputId, this)));
				InputId++;
			}
		}
	}
}

void APlayerCharacter::EquipGloves(const FItemInfo& Gloves)
{
	if (HasAuthority() && Gloves.ItemType == EItemType::Gloves)
	{
		if (bHasGloves)
			UnequipGloves();

		bHasGloves = true;
		GlovesInfo = Gloves;
		GlovesGivenStats = AddEquipmentStats(GlovesInfo.ItemStats);
	}
}

void APlayerCharacter::EquipBelt(const FItemInfo& Belt)
{
	if (HasAuthority() && Belt.ItemType == EItemType::Belt)
	{
		if (bHasBelt)
			UnequipBelt();

		bHasBelt = true;
		BeltInfo = Belt;
		BeltGivenStats = AddEquipmentStats(BeltInfo.ItemStats);
	}
}

void APlayerCharacter::EquipRing(const FItemInfo& Ring)
{
	if (HasAuthority() && Ring.ItemType == EItemType::Ring)
	{
		if (bHasRing)
			UnequipRing();

		bHasRing = true;
		RingInfo = Ring;
		RingGivenStats = AddEquipmentStats(RingInfo.ItemStats);
	}
}

void APlayerCharacter::EquipNecklace(const FItemInfo& Necklace)
{
	if (HasAuthority() && Necklace.ItemType == EItemType::Necklace)
	{
		if (bHasNecklace)
			UnequipNecklace();

		bHasNecklace = true;
		NecklaceInfo = Necklace;
		NecklaceGivenStats = AddEquipmentStats(NecklaceInfo.ItemStats);
	}
}

void APlayerCharacter::EquipEarrings(const FItemInfo& Earrings)
{
	if (HasAuthority() && Earrings.ItemType == EItemType::Earrings)
	{
		if (bHasEarrings)
			UnequipEarrings();

		bHasEarrings = true;
		EarringsInfo = Earrings;
		EarringsGivenStats = AddEquipmentStats(EarringsInfo.ItemStats);
	}
}

void APlayerCharacter::UnequipWeaponKit(const bool bWillEquipAfter)
{
	if (HasAuthority())
	{
		if (IsValid(RightHandWeaponActor)) RightHandWeaponActor->Destroy();
		if (IsValid(LeftHandWeaponActor)) LeftHandWeaponActor->Destroy();

		for (auto Ability : WeaponKitGivenAbilities)
			AbilitySystemComponent->ClearAbility(Ability);

		WeaponKitGivenAbilities.Empty();

		for (auto Ability : WeaponKitBasicAttacksGivenAbilities)
			AbilitySystemComponent->ClearAbility(Ability);

		WeaponKitBasicAttacksGivenAbilities.Empty();

		AbilitySystemComponent->RemoveActiveGameplayEffect(WeaponGivenStats);

		if (!bWillEquipAfter)
		{
			WeaponKitInfo = {};
			WeaponKitInfo.RowName = FName("NoWeapon");
			WeaponKitInfo.ItemType = EItemType::Weapon;
			WeaponKitRow = *CastChecked<UEfhorisGameInstance>(GetWorld()->GetGameInstance())->GetWeaponKitsDataTable()->FindRow<FWeaponKitRow>(WeaponKitInfo.RowName, {});
			bHasWeaponKit = false;
			NumberOfBasicAttacks = 0;

			if (IsValid(DefaultDodgeAbility))
				AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(DefaultDodgeAbility, 1, static_cast<int32>(DefaultDodgeAbility.GetDefaultObject()->GetAbilityInputID()), this));
		}
	}
}

void APlayerCharacter::UnequipGloves()
{
	if (HasAuthority())
	{
		bHasGloves = false;
		GlovesInfo = {};
		GlovesInfo.ItemType = EItemType::Gloves;
		AbilitySystemComponent->RemoveActiveGameplayEffect(GlovesGivenStats);
	}
}

void APlayerCharacter::UnequipBelt()
{
	if (HasAuthority())
	{
		bHasBelt = false;
		BeltInfo = {};
		BeltInfo.ItemType = EItemType::Belt;
		AbilitySystemComponent->RemoveActiveGameplayEffect(BeltGivenStats);
	}
}

void APlayerCharacter::UnequipRing()
{
	if (HasAuthority())
	{
		bHasRing = false;
		RingInfo = {};
		RingInfo.ItemType = EItemType::Ring;
		AbilitySystemComponent->RemoveActiveGameplayEffect(RingGivenStats);
	}
}

void APlayerCharacter::UnequipNecklace()
{
	if (HasAuthority())
	{
		bHasNecklace = false;
		NecklaceInfo = {};
		NecklaceInfo.ItemType = EItemType::Necklace;
		AbilitySystemComponent->RemoveActiveGameplayEffect(NecklaceGivenStats);
	}
}

void APlayerCharacter::UnequipEarrings()
{
	if (HasAuthority())
	{
		bHasEarrings = false;
		EarringsInfo = {};
		EarringsInfo.ItemType = EItemType::Earrings;
		AbilitySystemComponent->RemoveActiveGameplayEffect(EarringsGivenStats);
	}
}

void APlayerCharacter::ToggleRightWeaponVisibility() const
{
	if (IsValid(RightHandWeaponActor))
		RightHandWeaponActor->SetActorHiddenInGame(!RightHandWeaponActor->IsHidden());
}

void APlayerCharacter::ToggleLeftWeaponVisibility() const
{
	if (IsValid(LeftHandWeaponActor))
		LeftHandWeaponActor->SetActorHiddenInGame(!LeftHandWeaponActor->IsHidden());
}

void APlayerCharacter::SetLeftWeaponVisibility(const bool bIsHidden) const {
	if (IsValid(LeftHandWeaponActor))
		LeftHandWeaponActor->SetActorHiddenInGame(bIsHidden);
}

void APlayerCharacter::OnRep_HasSmithItem()
{
	if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
		if (AEfhorisPlayerController* PlayerController = CastChecked<AEfhorisPlayerController>(World->GetFirstPlayerController()); ensure(IsValid(PlayerController)))
			PlayerController->Client_RefreshOpenedPanels();
}

bool APlayerCharacter::IsInteractible(const APlayerCharacter* Player) const {
	const FGameplayTag DyingTag = FGameplayTag::RequestGameplayTag(FName("Character.Status.IsDying"));
	return Player->IsAliveTag() && AbilitySystemComponent->HasMatchingGameplayTag(DyingTag) && PlayerHealingMe == nullptr;
}

void APlayerCharacter::StartMercenaryReviveMe(AMercenary* Mercenary)
{
	FGameplayEventData GameplayEventData;
	GameplayEventData.Instigator = Mercenary;
	GameplayEventData.Target = this;
	GameplayEventData.ContextHandle = AbilitySystemComponent->MakeEffectContext();

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Interact.Start")), GameplayEventData);

	Mercenary->SetPlayerBeingRevived(this);
}

void APlayerCharacter::StopMercenaryReviveMe(AMercenary* Mercenary)
{
	FGameplayEventData GameplayEventData;
	GameplayEventData.Instigator = Mercenary;
	GameplayEventData.Target = this;
	GameplayEventData.ContextHandle = AbilitySystemComponent->MakeEffectContext();

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Interact.Stop")), GameplayEventData);

	Mercenary->SetPlayerBeingRevived(nullptr);
}

void APlayerCharacter::Server_SetIsSmithing_Implementation(bool IsSmithing) {
	bIsSmithing = IsSmithing;
}

void APlayerCharacter::Server_SetIsTrading_Implementation(bool IsTrading) {
	bIsTrading = IsTrading;
}

void APlayerCharacter::Server_SetIsInteracting_Implementation(bool IsInteracting) {
	bIsInteracting = IsInteracting;
}

void APlayerCharacter::Server_SetAllInteractionBooleans_Implementation(bool IsInteracting, bool IsTrading, bool IsSmithing) {
	bIsInteracting = IsInteracting;
	bIsTrading = IsTrading;
	bIsSmithing = IsSmithing;
}

void APlayerCharacter::StartInteract(APlayerCharacter* Player)
{
	PlayerHealingMe = Player;
	if (IsValid(PlayerHealingMe))
		PlayerHealingMe->Server_AddInteraction();
	FGameplayEventData GameplayEventData;
	GameplayEventData.Instigator = Player;
	GameplayEventData.Target = this;
	GameplayEventData.ContextHandle = AbilitySystemComponent->MakeEffectContext();

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Interact.Start")), GameplayEventData);
}

void APlayerCharacter::StopInteract(APlayerCharacter* Player)
{
	if (IsValid(PlayerHealingMe))
		PlayerHealingMe->Server_RemoveInteraction();
	PlayerHealingMe = nullptr;
	FGameplayEventData GameplayEventData;
	GameplayEventData.Instigator = Player;
	GameplayEventData.Target = this;
	GameplayEventData.ContextHandle = AbilitySystemComponent->MakeEffectContext();

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, FGameplayTag::RequestGameplayTag(FName("Character.Action.Base.Interact.Stop")), GameplayEventData);
}

const FText APlayerCharacter::GetInteractionText() const
{
	return InteractionText;
}

void APlayerCharacter::Server_TryBuyItem_Implementation(AMerchant* Merchant, FItemInfo ItemInfo)
{
	if (AEfhorisGameMode* GameMode = GetWorld()->GetAuthGameMode<AEfhorisGameMode>())
		GameMode->TryBuyItem(Merchant, ItemInfo, this);
}

void APlayerCharacter::Server_TrySellItem_Implementation(AMerchant* Merchant, FItemInfo ItemInfo)
{
	if (AEfhorisGameMode* GameMode = GetWorld()->GetAuthGameMode<AEfhorisGameMode>())
		GameMode->TrySellItem(Merchant, ItemInfo, this);
}

AEfhorisGameState* APlayerCharacter::GetGameState() const
{
	return GetWorld()->GetGameState<AEfhorisGameState>();
}

void APlayerCharacter::Die()
{
	Super::Die();

	GetInteractionComponent()->Server_ForceStopInteract();

	if (const AAIManager* Manager = GetWorld()->GetGameState<AEfhorisGameState>()->GetAIManager(); IsValid(Manager)) {
		Manager->ClearEnemiesThreat(this);
	}

	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetController()); ensure(IsValid(PlayerController)))
	{
		PlayerController->Client_ForceCloseAllMenus(true);
	}
}

float APlayerCharacter::GetPotionCharge() const
{
	return PotionAttributes->GetPotionCharge();
}

float APlayerCharacter::GetMaxPotionCharge() const
{
	return PotionAttributes->GetMaxPotionCharge();
}

void APlayerCharacter::Regen()
{
	HealthAttributes->SetHealth(GetMaxHealth());
	PotionAttributes->SetPotionCharge(GetMaxPotionCharge());

	AbilitySystemComponent->CancelAllAbilities();
}

void APlayerCharacter::SetPotionCharge(int Charges)
{
	PotionAttributes->SetPotionCharge(Charges);
}


void APlayerCharacter::Multicast_SendReviveEvent_Implementation()
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.Revive")), {});
}

FActiveGameplayEffectHandle APlayerCharacter::AddEquipmentStats(const FItemStats ItemStats) const
{
	FGameplayEffectSpec GESpec{ EquipmentEffect.GetDefaultObject(), {} };

	GESpec.SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Character.Statistics.MaxHealth")), ItemStats.MaxHealth);
	GESpec.SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Character.Statistics.MaxSpeed")), ItemStats.MovementSpeed);
	GESpec.SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Character.Statistics.MaxPotionCharge")), ItemStats.MaxPotionCharge);
	GESpec.SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Character.Statistics.PhysicalPower")), ItemStats.PhysicalPower);
	GESpec.SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Character.Statistics.PhysicalDefense")), ItemStats.PhysicalDefense);
	GESpec.SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Character.Statistics.BlockingPhysicalDefense")), ItemStats.BlockingPhysicalDefense);
	GESpec.SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Character.Statistics.MagicalPower")), ItemStats.MagicalPower);
	GESpec.SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Character.Statistics.MagicalDefense")), ItemStats.MagicalDefense);
	GESpec.SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Character.Statistics.BlockingMagicalDefense")), ItemStats.BlockingMagicalDefense);

	return AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(GESpec);
}

void APlayerCharacter::Server_UnequipKitFromPlayer_Implementation(APlayerCharacter* Character, FItemInfo ItemInfo)
{
	if (HasAuthority())
	{
		if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
		{
			if (AEfhorisGameMode* GameMode = Cast<AEfhorisGameMode>(World->GetAuthGameMode()); ensure(IsValid(GameMode)))
			{
				GameMode->UnequipKitFromPlayer(Character, ItemInfo);
			}
		}
	}
}

void APlayerCharacter::Server_EquipKitToPlayer_Implementation(APlayerCharacter* Character, FItemInfo ItemInfo)
{
	if(HasAuthority())
	{
		if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
		{
			if (AEfhorisGameMode* GameMode = Cast<AEfhorisGameMode>(World->GetAuthGameMode()); ensure(IsValid(GameMode)))
			{
				if (!Character->AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Action"))))
				{
					GameMode->EquipKitToPlayer(Character, ItemInfo);
				}
			}
		}
	}
}


void APlayerCharacter::Server_TrySetSmithItem_Implementation(FItemInfo ItemInfo)
{
	if (HasAuthority())
	{
		if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
		{
			if (AEfhorisGameMode* GameMode = Cast<AEfhorisGameMode>(World->GetAuthGameMode()); ensure(IsValid(GameMode)))
			{
				GameMode->SetSmithItemToPlayer(this, ItemInfo);
			}
		}
	}
}

void APlayerCharacter::Server_TryRemoveSmithItem_Implementation()
{
	if (HasAuthority())
	{
		if (const UWorld* World = GetWorld(); ensure(IsValid(World)))
		{
			if (AEfhorisGameMode* GameMode = Cast<AEfhorisGameMode>(World->GetAuthGameMode()); ensure(IsValid(GameMode)))
			{
				GameMode->RemoveSmithItemFromPlayer(this);
			}
		}
	}
}

#pragma region Mercenary

TArray<AMercenary*>& APlayerCharacter::GetMercenaries() {
	return Mercenaries;
}

void APlayerCharacter::AddMercenary(AMercenary* Mercenary) {
	Mercenaries.Add(Mercenary);
	if (const auto MasterController = GetController<AEfhorisPlayerController>(); IsValid(MasterController))
		MasterController->Client_ShowFloatingHealthBar(Mercenary, true);
}

void APlayerCharacter::RemoveMercenary(AMercenary* Mercenary) {
	Mercenaries.Remove(Mercenary);
	if (const auto MasterController = GetController<AEfhorisPlayerController>(); IsValid(MasterController))
		MasterController->Client_ShowFloatingHealthBar(Mercenary, false);
}

void APlayerCharacter::MercenariesFindNewMaster() {
	while (!Mercenaries.IsEmpty())
	{
		AMercenary* Mercenary = Mercenaries[0];
		if (IsValid(Mercenary)) {
			Mercenary->FindNewMaster();
		}
		else {
			Mercenaries.Remove(Mercenary);
		}
	}
}

#pragma endregion