#include "PickableItem.h"

#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"

void APickableItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APickableItem, ItemInfo);
}

APickableItem::APickableItem()
{
	PrimaryActorTick.bCanEverTick = true;
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Component"));
	BoxComponent->SetIsReplicated(true);
	BoxComponent->SetupAttachment(SceneComponent);

	StaticMeshComponent->SetupAttachment(BoxComponent);

	GroundFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Ground FX Component"));
	GroundFX->SetupAttachment(BoxComponent);

	BeamFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Beam FX Component"));
	BeamFX->SetupAttachment(BoxComponent);
}

void APickableItem::BeginPlay()
{
	Super::BeginPlay();
	InitialTransform = StaticMeshComponent->GetRelativeTransform();
	ActivateFX(false);

	GroundFX->SetRelativeLocation(FVector(0, 0, 1 - BoxComponent->GetScaledBoxExtent().Z));
}

void APickableItem::Tick(float DeltaSeconds)
{
	if (!bIsAnimated)
	{
		//GEngine->DrawOnscreenDebugMessages(GetWorld(),)
		if (BoxComponent->GetPhysicsLinearVelocity().IsZero()) {
			StartAnimation();
		}
	}
	if (!HasAuthority() && bIsAnimated)
	{
		Super::Tick(DeltaSeconds);
		//Play animation
		Time += DeltaSeconds;
		const FVector Translation = FVector(0, 0, AnimationAmplitude * (1 + FMath::Sin(Time * AnimationFrequency)));
		const FRotator Rotation = FRotator(0, Time * AnimationRotationSpeed, 0) + InitialTransform.Rotator();

		const FVector AnimatedLocation = InitialTransform.GetLocation() + Translation;

		BeamFX->SetRelativeLocation(Translation);

		StaticMeshComponent->SetRelativeLocation(AnimatedLocation);
		StaticMeshComponent->SetRelativeRotation(Rotation);
	}
}

FLinearColor APickableItem::GetColorLevel(int Level) const {
	if (ColorLevel.IsEmpty()) return FLinearColor(1, 1, 1, EmissionIntensity);
	if (ItemInfo.ItemType == EItemType::Trash) return ColorLevel[0];

	const int ClampedLevel = FMath::Clamp(Level, 0, ColorLevel.Num()-1);
	auto Color = ColorLevel[ClampedLevel];
	Color.A = EmissionIntensity;
	return Color;
}

void APickableItem::SetFXColor() const {
	const FLinearColor Back = FLinearColor(0, 0, 0, EmissionIntensity);
	const FLinearColor Emission = GetColorLevel(ItemInfo.Level);

	BeamFX->SetVariableLinearColor("User.EmissionColor", Emission);
	BeamFX->SetVariableLinearColor("User.BackColor", Back);

	GroundFX->SetVariableLinearColor("User.EmissionColor", Emission);
	GroundFX->SetVariableLinearColor("User.BackColor", Back);
}

void APickableItem::ActivateFX(const bool bIsActive) {
	if (bIsActive)
	{
		BeamFX->Activate(true);
		GroundFX->Activate(true);
	}
	else
	{
		BeamFX->Deactivate();
		GroundFX->Deactivate();
	}
}

void APickableItem::StartAnimation() {
	ActivateFX(true);
	bIsAnimated = true;
	SetFXColor();
}

FItemInfo& APickableItem::GetItemInfo() noexcept
{
	return ItemInfo;
}

void APickableItem::SetItemInfo(const FItemInfo& NewItemInfo) noexcept
{
	ItemInfo = NewItemInfo;
}

UBoxComponent* APickableItem::GetCollision() const noexcept {
	return BoxComponent;
}

bool APickableItem::IsPickable() const noexcept
{
	return bIsPickable;
}

void APickableItem::SetIsPickable(const bool bNewIsPickable) noexcept
{
	bIsPickable = bNewIsPickable;
}
