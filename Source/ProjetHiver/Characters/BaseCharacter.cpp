#include "BaseCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AttributeSets/HealthAttributeSet.h"
#include "AttributeSets/MovementAttributeSet.h"
#include "AttributeSets/PowerAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "ProjetHiver/AI/Signals/SignalComponent.h"
#include "ProjetHiver/GAS/EfhorisAbilitySystemComponent.h"
#include "ProjetHiver/GAS/EfhorisGameplayAbility.h"
#include "AkComponent.h"

// Sets default values
ABaseCharacter::ABaseCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Our ability system component.
	AbilitySystemComponent = CreateDefaultSubobject<UEfhorisAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Create the signal component
	SignalComponent = CreateDefaultSubobject<USignalComponent>(TEXT("SignalComponent"));
	SignalComponent->SetupAttachment(RootComponent);

	// Create the sound component
	//AkComponent = CreateDefaultSubobject<UAkComponent>(TEXT("AkComponent"));
	//AkComponent->SetupAttachment(GetMesh());

	HealthAttributes = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthAttributes"));
	MovementAttributes = CreateDefaultSubobject<UMovementAttributeSet>(TEXT("MovementAttributes"));
	PowerAttributes = CreateDefaultSubobject<UPowerAttributeSet>(TEXT("PowerAttributes"));
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseCharacter, CharacterLevel);
}

void ABaseCharacter::InitializeAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

void ABaseCharacter::InitializeAbilities()
{
	if (HasAuthority() && IsValid(AbilitySystemComponent))
	{
		for (TSubclassOf<UEfhorisGameplayAbility>& DefaultAbility : DefaultAbilities)
			if (IsValid(DefaultAbility))
				AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(DefaultAbility, 1, static_cast<int32>(DefaultAbility.GetDefaultObject()->GetAbilityInputID()), this));
	}
}

void ABaseCharacter::InitializeAttributes()
{
	if (!IsValid(AbilitySystemComponent)) return;

	if (!DefaultAttributes)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Missing DefaultAttributes for %s. Please fill in the character's Blueprint."), *FString(__FUNCTION__), *GetName());
		return;
	}

	// Can run on Server and Client
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	const FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributes, CharacterLevel, EffectContext);
	if (NewHandle.IsValid())
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
}

void ABaseCharacter::PerformMeleeHitboxTrace()
{
	if (HasAuthority())
	{
		const auto SpherePos = GetActorLocation() + GetActorRotation().RotateVector(MeleeSphereOffset);

		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel4));

		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(this);

		for (auto& Actor : MeleeHitTargets)
			ActorsToIgnore.Add(Actor.Get());

		TArray<AActor*> OutActors;

		FColor Color;

		if (UKismetSystemLibrary::SphereOverlapActors(GetWorld(), SpherePos, MeleeSphereRadius, ObjectTypes, StaticClass(), ActorsToIgnore, OutActors))
		{
			for (AActor* Actor : OutActors)
			{
				ABaseCharacter* HitActor = Cast<ABaseCharacter>(Actor);

				if (HitActor &&
					HitActor->FactionTag != FactionTag &&
					!HitActor->GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsInvincible"))))
				{
					MeleeHitTargets.Add(HitActor);

					FGameplayEventData GameplayEventData{};

					bool bSuccessfulParry = false;

					if (HitActor->GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Action.Combat.SwordShield.Parry"))))
						bSuccessfulParry = HitActor->IsSuccessfullyBlocking(this);

					if (bSuccessfulParry)
					{
						GameplayEventData.Instigator = HitActor;
						GameplayEventData.Target = this;
						GameplayEventData.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(this);

						UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(HitActor, FGameplayTag::RequestGameplayTag(FName("Character.AbilityEvent.Parry")), GameplayEventData);
					}
					else
					{
						GameplayEventData.Instigator = this;
						GameplayEventData.Target = HitActor;
						GameplayEventData.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitActor);

						UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, FGameplayTag::RequestGameplayTag(FName("Character.Animation.MeleeStrike")), GameplayEventData);
					}
				}
			}

			Color = FColor::Green;
		}
		else
			Color = FColor::Red;

		if (bDebugAbilitiesHitbox)
			DrawDebugSphere(GetWorld(), SpherePos, MeleeSphereRadius, 10, Color, false, 3.f);
	}
}

void ABaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (HasAuthority() && IsValid(AbilitySystemComponent) && IsValid(GroundedEffect))
	{
		if (AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsGrounded")))) return;

		const FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GroundedEffect, 1, EffectContext);

		if (SpecHandle.IsValid())
			GroundedEffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		bIsBumped = false;
	}
}

void ABaseCharacter::Falling()
{
	Super::Falling();

	if (HasAuthority() && IsValid(AbilitySystemComponent) && IsValid(GroundedEffect))
		if (GroundedEffectHandle.IsValid())
			AbilitySystemComponent->RemoveActiveGameplayEffect(GroundedEffectHandle);
}

void ABaseCharacter::ResetMeleeHitTargets()
{
	MeleeHitTargets.Empty();
}

void ABaseCharacter::ForceCharacterRotation()
{
	if(HasAuthority())
	{
		auto Rotation = GetActorRotation();
		Rotation.Yaw = GetControlRotation().Yaw;

		SetActorRotation(Rotation);
	}
}

UAbilitySystemComponent* ABaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UHealthAttributeSet* ABaseCharacter::GetHealthAttributeSet() const {
	return HealthAttributes;
}

bool ABaseCharacter::IsStunned() const
{
	return AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsStunned")));
}

bool ABaseCharacter::IsAliveTag() const noexcept
{
	FGameplayTagContainer DeathTags;
	DeathTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsDying")));
	DeathTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.IsDead")));

	return !AbilitySystemComponent->HasAnyMatchingGameplayTags(DeathTags);
}

bool ABaseCharacter::IsAliveHealth() const noexcept
{
	return GetHealth() > 0.0f;
}

void ABaseCharacter::Die()
{
	const FGameplayTagContainer Tag{ FGameplayTag::RequestGameplayTag(FName("Character.Die")) };
	AbilitySystemComponent->TryActivateAbilitiesByTag(Tag);
}

float ABaseCharacter::GetHealth() const
{
	return HealthAttributes->GetHealth();
}

float ABaseCharacter::GetMaxHealth() const
{
	return HealthAttributes->GetMaxHealth();
}

void ABaseCharacter::SetHealthPercentage(float HealthPercentage)
{
	HealthAttributes->SetHealth(GetMaxHealth() * HealthPercentage);
}

void ABaseCharacter::SetMaxHealth(const float _MaxHealth) const
{
	HealthAttributes->SetMaxHealth(_MaxHealth);
}

float ABaseCharacter::GetDamage() const
{
	return HealthAttributes->GetDamage();
}

float ABaseCharacter::GetMaxSpeed() const
{
	return MovementAttributes->GetMaxSpeed();
}

float ABaseCharacter::GetPhysicalPower() const
{
	return PowerAttributes->GetPhysicalPower();
}

float ABaseCharacter::GetPhysicalDefense() const
{
	return PowerAttributes->GetPhysicalDefense();
}

float ABaseCharacter::GetBlockingPhysicalDefense() const
{
	return PowerAttributes->GetBlockingPhysicalDefense();
}

float ABaseCharacter::GetMagicalPower() const
{
	return PowerAttributes->GetMagicalPower();
}

float ABaseCharacter::GetMagicalDefense() const
{
	return PowerAttributes->GetMagicalDefense();
}

float ABaseCharacter::GetBlockingMagicalDefense() const
{
	return PowerAttributes->GetBlockingMagicalDefense();
}

void ABaseCharacter::SetMeleeSphereRadius(const float Radius) noexcept
{
	MeleeSphereRadius = Radius;
}

void ABaseCharacter::SetMeleeSphereOffset(const FVector& Offset)
{
	MeleeSphereOffset = Offset;
}

int ABaseCharacter::GetCharacterLevel() const noexcept
{
	return CharacterLevel;
}

void ABaseCharacter::SetCharacterLevel(int Level) noexcept
{
	CharacterLevel = Level;
}

void ABaseCharacter::Multicast_SetCollision_Implementation(const ECollisionEnabled::Type CollisionType) {
	GetCapsuleComponent()->SetCollisionEnabled(CollisionType);
}

bool ABaseCharacter::IsSuccessfullyBlocking(const AActor* Attacker) const
{
	const auto DefenderForwardVector = GetTransform().GetRotation().GetForwardVector();
	const auto AttackerToDefenderVector = (Attacker->GetTransform().GetLocation() - GetTransform().GetLocation());

	return AttackerToDefenderVector.CosineAngle2D(DefenderForwardVector) >= 0.f;
}

bool ABaseCharacter::GetDebugAbilitiesHitbox() const noexcept
{
	return bDebugAbilitiesHitbox;
}

bool ABaseCharacter::IsBumped() const noexcept
{
	return bIsBumped;
}

void ABaseCharacter::SetIsBumped(const bool _bIsBumped) noexcept
{
	bIsBumped = _bIsBumped;
}

FGameplayTag ABaseCharacter::GetFactionTag() const {
	return FactionTag;
}

bool ABaseCharacter::IsAlly() const {
	const FGameplayTag AllyTag = FGameplayTag::RequestGameplayTag(FName("Character.Faction.Player"));
	return FactionTag == AllyTag;
}

float ABaseCharacter::GetDamageNumberOffset() const {
	return DamageNumberOffset;
}

void ABaseCharacter::Suicide() const {
		UGameplayEffect* GESuicide = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("Suicide")));
		GESuicide->DurationPolicy = EGameplayEffectDurationType::Instant;

		GESuicide->Modifiers.SetNum(1);

		FGameplayModifierInfo& RemoveHealth = GESuicide->Modifiers[0];
		RemoveHealth.ModifierMagnitude = FScalableFloat(999999);
		RemoveHealth.ModifierOp = EGameplayModOp::Additive;
		RemoveHealth.Attribute = HealthAttributes->GetDamageAttribute();

		AbilitySystemComponent->ApplyGameplayEffectToSelf(GESuicide, 1.0f, AbilitySystemComponent->MakeEffectContext());

}
