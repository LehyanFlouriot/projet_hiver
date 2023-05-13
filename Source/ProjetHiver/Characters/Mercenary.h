#pragma once

#include "CoreMinimal.h"
#include "BaseNPC.h"
#include "AttributeSets/PotionAttributeSet.h"
#include "ProjetHiver/Interaction/InteractiveActor.h"
#include "Mercenary.generated.h"

class APlayerCharacter;
class UFloatingHealthBar;
class UWidgetComponent;
class UPickUpComponent;
class UPotionAttributeSet;
class AReplicatedStaticMeshActor;

UCLASS()
class PROJETHIVER_API AMercenary final : public ABaseNPC, public IInteractiveActor
{
	GENERATED_BODY()

protected:
	/** Component allowing the player to pick-up items */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	UPickUpComponent* PickUpComponent;

	UPROPERTY(EditAnywhere, Category = GAS)
	TSubclassOf<UEfhorisGameplayAbility> DefaultDodgeAbility;

	UPROPERTY(EditAnywhere)
	int MoneyCost = 100;

	UPROPERTY(EditAnywhere)
	float MoneyScaling = 0.25;

	UPROPERTY(Transient,Replicated)
	APlayerCharacter* Master = nullptr;

	FString MasterId;

	UPROPERTY(EditAnywhere, NoClear)
	TSubclassOf<AReplicatedStaticMeshActor> WeaponMesh;

	UPROPERTY(Transient)
	AReplicatedStaticMeshActor* BackWeapon;

	UPROPERTY(Transient)
	AReplicatedStaticMeshActor* RightHandWeapon;

	UPROPERTY(EditAnywhere, NoClear)
	UAnimMontage* EquipMontage;

	UPROPERTY(EditAnywhere, NoClear)
	UAnimMontage* UnequipMontage;

	UPROPERTY(EditAnywhere)
	FText InteractionTextRecruit;

	UPROPERTY(EditAnywhere)
	FString PlaceHolderMoney;

	UPROPERTY(EditAnywhere)
	FText InteractionTextChangeOwner;

	UPROPERTY(EditAnywhere)
	FColor HealthColor = FColor(0, 255, 0);

	UPROPERTY(EditAnywhere)
	bool bShowIMap = false;

	virtual void Die() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void ClearUnitReferences() override;

public:
	AMercenary(const FObjectInitializer& ObjectInitializer);

	virtual bool IsInteractible(const APlayerCharacter* Player) const override;
	virtual void StartInteract(APlayerCharacter* Player) override;
	virtual void StopInteract(APlayerCharacter* Player) override;
	virtual const FText GetInteractionText() const override;

	UFUNCTION()
	void SetBackWeaponVisibility(const bool bIsHidden) const;

	UFUNCTION()
	void SetHandWeaponVisibility(const bool bIsHidden) const;

	UFUNCTION(BlueprintCallable)
	void SetWeaponVisibility(const bool bIsEquipped) const;

	UFUNCTION()
	void Regen();

	UFUNCTION(BlueprintCallable)
	bool HasMaster() const noexcept;

	UFUNCTION()
	void SetMaster(APlayerCharacter* NewMaster);

	UFUNCTION(BlueprintCallable)
	APlayerCharacter* GetMaster() const noexcept;

	void SetMasterId(const FString& Id) noexcept { MasterId = Id; }

	const FString& GetMasterId() const noexcept { return MasterId; }

	bool HasMasterId() const noexcept { return !MasterId.IsEmpty(); }

	UFUNCTION()
	void FindNewMaster();

	UFUNCTION(BlueprintCallable)
	FVector LocationAroundMaster();

	UFUNCTION()
	void Authority_Teleport(const FVector& Position);

	UFUNCTION(BlueprintCallable)
	bool IsRecruted() const noexcept;

	UFUNCTION(BlueprintCallable)
	void TryCancelCombo();

	UFUNCTION(BlueprintCallable)
	void CancelCombo();

#pragma region Healing
protected:
	UPROPERTY(Transient)
	UPotionAttributeSet* PotionAttributes;

	UFUNCTION(BlueprintCallable)
	FVector FindHealingLocation();

public:
	UFUNCTION(BlueprintCallable)
	float GetPotionCharge() const;

	UFUNCTION()
	void SetPotionCharge(float PotionCharge_) noexcept { PotionAttributes->PotionCharge = PotionCharge_; }

	UFUNCTION()
	float GetMaxPotionCharge() const;

	UFUNCTION()
	void SetMaxPotionCharge(float MaxPotionCharge_) noexcept { PotionAttributes->MaxPotionCharge = MaxPotionCharge_; }

	UFUNCTION(BlueprintCallable)
	bool ShouldHeal(const float Threshold) const;
#pragma endregion

#pragma region RevivePlayer
protected:
	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<APlayerCharacter*> PlayersToRevive;

	UPROPERTY(Transient, BlueprintReadOnly)
	APlayerCharacter* PlayerBeingRevived;

	UFUNCTION()
	void StopCurrentReviveAttempt();

public:
	TArray<APlayerCharacter*>& GetPlayersToRevive() noexcept { return PlayersToRevive; }

	UFUNCTION(BlueprintCallable)
	void AddPlayerToRevive(APlayerCharacter* PlayerToRevive) noexcept { PlayersToRevive.Add(PlayerToRevive); }

	UFUNCTION(BlueprintCallable)
	void DeletePlayerToRevive(APlayerCharacter* PlayerToRevive) noexcept;

	UFUNCTION(BlueprintCallable)
	APlayerCharacter* GetPlayerBeingRevived() const noexcept { return PlayerBeingRevived; }

	UFUNCTION(BlueprintCallable)
	void SetPlayerBeingRevived(APlayerCharacter* PlayerBeingRevived_) noexcept { PlayerBeingRevived = PlayerBeingRevived_; }

	UFUNCTION()
	void ClearAllRevive();

#pragma endregion
};