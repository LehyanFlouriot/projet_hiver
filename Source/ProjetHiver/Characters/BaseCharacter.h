#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

class UEfhorisAbilitySystemComponent;
class UEfhorisGameplayAbility;
class UGameplayEffect;
class UHealthAttributeSet;
class UMovementAttributeSet;
class UPowerAttributeSet;
class USignalComponent;
class UAkComponent;

UCLASS()
class PROJETHIVER_API ABaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Abilities, meta = (AllowPrivateAccess = "true"))
	UEfhorisAbilitySystemComponent* AbilitySystemComponent;

	/** Component allowing the player to send signals */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	USignalComponent* SignalComponent = nullptr;

	/** Component allowing the player to play sounds */
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	//UAkComponent* AkComponent;

	UPROPERTY(Transient)
	UHealthAttributeSet* HealthAttributes;

	UPROPERTY(Transient)
	UMovementAttributeSet* MovementAttributes;

	UPROPERTY(Transient)
	UPowerAttributeSet* PowerAttributes;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = GAS)
	TArray<TSubclassOf<UEfhorisGameplayAbility>> DefaultAbilities;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = GAS)
	TSubclassOf<UGameplayEffect> DefaultAttributes;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = GAS)
	TSubclassOf<UGameplayEffect> GroundedEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Faction)
	FGameplayTag FactionTag;

	UPROPERTY(Replicated)
	int CharacterLevel = 1;

	UPROPERTY(Transient)
	float MeleeSphereRadius;

	UPROPERTY(Transient)
	FVector MeleeSphereOffset;

	UPROPERTY(Transient)
	FActiveGameplayEffectHandle GroundedEffectHandle;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AActor>> MeleeHitTargets;

	UPROPERTY(EditAnywhere)
	bool bDebugAbilitiesHitbox;

	UPROPERTY(Transient)
	bool bIsBumped;

	UPROPERTY(EditAnywhere)
	float DamageNumberOffset = 0.0f;

public:
	// Sets default values for this character's properties
	ABaseCharacter(const FObjectInitializer& ObjectInitializer);

	void InitializeAbilityActorInfo();

protected:
	UFUNCTION(BlueprintCallable)
	void InitializeAbilities();
	UFUNCTION(BlueprintCallable)
	void InitializeAttributes();

public:
	UFUNCTION(BlueprintCallable)
	void PerformMeleeHitboxTrace();

	virtual void Landed(const FHitResult& Hit) override;
	virtual void Falling() override;

	UFUNCTION(BlueprintCallable)
	void ResetMeleeHitTargets();

	UFUNCTION(BlueprintCallable)
	void ForceCharacterRotation();

	UFUNCTION()
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override final;

	/** Returns Signal Component sub-object **/
	FORCEINLINE USignalComponent* GetSignalComponent() const noexcept { return SignalComponent; }

	/** Returns Sound Component sub-object **/
	//FORCEINLINE UAkComponent* GetAkComponent() const noexcept { return AkComponent; }

	UFUNCTION(BlueprintCallable)
	UHealthAttributeSet* GetHealthAttributeSet() const;

	UFUNCTION(BlueprintCallable)
	bool IsStunned() const;

	UFUNCTION(BlueprintCallable)
	bool IsAliveTag() const noexcept;

	UFUNCTION(BlueprintCallable)
	bool IsAliveHealth() const noexcept;

	UFUNCTION()
	virtual void Die();

	UFUNCTION(BlueprintCallable)
	float GetHealth() const;

	UFUNCTION(BlueprintCallable)
	float GetMaxHealth() const;

	UFUNCTION()
	void SetHealthPercentage(float HealthPercentage);

	UFUNCTION()
	void SetMaxHealth(const float _MaxHealth) const;

	UFUNCTION(BlueprintCallable)
	float GetDamage() const;

	UFUNCTION(BlueprintCallable)
	float GetMaxSpeed() const;

	UFUNCTION(BlueprintCallable)
	float GetPhysicalPower() const;

	UFUNCTION(BlueprintCallable)
	float GetPhysicalDefense() const;

	UFUNCTION(BlueprintCallable)
	float GetBlockingPhysicalDefense() const;

	UFUNCTION(BlueprintCallable)
	float GetMagicalPower() const;

	UFUNCTION(BlueprintCallable)
	float GetMagicalDefense() const;

	UFUNCTION(BlueprintCallable)
	float GetBlockingMagicalDefense() const;

	UFUNCTION(BlueprintCallable)
	int GetCharacterLevel() const noexcept;

	UFUNCTION()
	void SetCharacterLevel(int Level) noexcept;

	UFUNCTION()
	void SetMeleeSphereRadius(const float Radius) noexcept;

	UFUNCTION()
	void SetMeleeSphereOffset(const FVector& Offset);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SetCollision(const ECollisionEnabled::Type CollisionType);

	UFUNCTION()
	bool IsSuccessfullyBlocking(const AActor* Attacker) const;

	UFUNCTION()
	bool GetDebugAbilitiesHitbox() const noexcept;

	UFUNCTION()
	bool IsBumped() const noexcept;

	UFUNCTION()
	void SetIsBumped(const bool _bIsBumped) noexcept;

	UFUNCTION(BlueprintCallable)
	FGameplayTag GetFactionTag() const;

	UFUNCTION()
	bool IsAlly() const;

	UFUNCTION()
	float GetDamageNumberOffset() const;

	UFUNCTION(BlueprintCallable)
	void Suicide() const;
};
