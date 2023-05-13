// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "InputActionValue.h"
#include "ProjetHiver/ProjetHiver.h"
#include "ProjetHiver/Interaction/InteractiveActor.h"
#include "ProjetHiver/Items/ItemInfo.h"
#include "ProjetHiver/Items/ItemRows/ArmorKitRow.h"
#include "ProjetHiver/Items/ItemRows/WeaponKitRow.h"
#include "ProjetHiver/Misc/PlayerOptions.h"
#include "PlayerCharacter.generated.h"

class AMercenary;
class AEnemy;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UInteractionComponent;
class UPickUpComponent;
class UPlayersStatsMeasuringComponent;
class UPotionAttributeSet;
class USpringArmComponent;
class UFloatingName;
class UWidgetComponent;
struct FGameplayAbilitySpecHandle;

/* Following is a macro to help building input handles */
#define CREATE_HANDLERS_FOR_ABILITY(ActionName) \
	void Handle##ActionName##Pressed() \
	{ \
		if (bIsInteracting) return; \
		if (!bCanCancelAbility) return; \
		SendLocalInputToASC(true, EGASAbilityInputID::##ActionName); \
	} \
	\
	void Handle##ActionName##Released() \
	{ \
		SendLocalInputToASC(false, EGASAbilityInputID::##ActionName); \
	}

class APlayerCharacter;

USTRUCT(BlueprintType)
struct FPlayerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	APlayerCharacter* Character = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FString NickName;
};

UCLASS(config=Game)
class APlayerCharacter final : public ABaseCharacter, public IInteractiveActor
{
	GENERATED_BODY()

public:
	friend class UEfhorisCheatManager;

protected:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

#pragma region InputActions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DodgeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PotionAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackComboAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RightClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Skill1Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Skill2Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CallInventoryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ConfirmAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CancelAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* OpenMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* OpenMainMenu;

#pragma endregion

	/** Component allowing the player to pick-up items */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	UPickUpComponent* PickUpComponent = nullptr;

	/** Component allowing the player to pick-up items */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	UInteractionComponent* InteractionComponent = nullptr;

	/** Component allowing the player to pick-up items */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	UPlayersStatsMeasuringComponent* PlayersStatsMeasuringComponent = nullptr;

	/** Allows component to set-up input joined with the player */
	DECLARE_MULTICAST_DELEGATE_OneParam(FSetupPlayerInput, UInputComponent*);
	FSetupPlayerInput OnSetupPlayerInput;

	UPROPERTY(Transient)
	FVector2D LastInputVector;

	UPROPERTY(Transient)
	UPotionAttributeSet* PotionAttributes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GAS)
	TSubclassOf<UEfhorisGameplayAbility> DefaultDodgeAbility;

	UPROPERTY(Transient)
	TArray<FGameplayAbilitySpecHandle> WeaponKitGivenAbilities;

	UPROPERTY(Transient)
	TArray<FGameplayAbilitySpecHandle> WeaponKitBasicAttacksGivenAbilities;

	UPROPERTY(Transient)
	TArray<FGameplayAbilitySpecHandle> BuffGivenAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GAS)
	TSubclassOf<UGameplayEffect> EquipmentEffect;

	UPROPERTY(EditAnywhere, Category = Armor)
	USkeletalMesh* DefaultRootMesh;

	UPROPERTY(EditAnywhere, Category = Armor)
	TArray<USkeletalMesh*> DefaultSubMeshes;

	UPROPERTY(Transient, BlueprintReadOnly, VisibleAnywhere)
	bool bUseControlRig;
#pragma region Inventory

	UPROPERTY(Replicated, Transient)
	bool bHasArmorKit;

	UPROPERTY(Replicated, Transient)
	bool bHasWeaponKit;

	UPROPERTY(Replicated, Transient)
	bool bHasGloves;

	UPROPERTY(Replicated, Transient)
	bool bHasBelt;

	UPROPERTY(Replicated, Transient)
	bool bHasRing;

	UPROPERTY(Replicated, Transient)
	bool bHasNecklace;

	UPROPERTY(Replicated, Transient)
	bool bHasEarrings;

	UPROPERTY(Transient)
	FActiveGameplayEffectHandle ArmorGivenStats;

	UPROPERTY(Transient)
	FActiveGameplayEffectHandle WeaponGivenStats;

	UPROPERTY(Transient)
	FActiveGameplayEffectHandle GlovesGivenStats;

	UPROPERTY(Transient)
	FActiveGameplayEffectHandle BeltGivenStats;

	UPROPERTY(Transient)
	FActiveGameplayEffectHandle RingGivenStats;

	UPROPERTY(Transient)
	FActiveGameplayEffectHandle NecklaceGivenStats;

	UPROPERTY(Transient)
	FActiveGameplayEffectHandle EarringsGivenStats;

	UPROPERTY(Replicated, Transient)
	FItemInfo ArmorKitInfo;

	UPROPERTY(Replicated, Transient, ReplicatedUsing=OnRep_RefreshSpellsPanelWidget)
	FItemInfo WeaponKitInfo;

	UPROPERTY(Replicated, Transient)
	FItemInfo GlovesInfo;

	UPROPERTY(Replicated, Transient)
	FItemInfo BeltInfo;

	UPROPERTY(Replicated, Transient)
	FItemInfo RingInfo;

	UPROPERTY(Replicated, Transient)
	FItemInfo NecklaceInfo;

	UPROPERTY(Replicated, Transient)
	FItemInfo EarringsInfo;

	UPROPERTY(Replicated, Transient, VisibleAnywhere, BlueprintReadOnly)
	FWeaponKitRow WeaponKitRow;

#pragma endregion

	UPROPERTY(Transient)
	TArray<USkeletalMeshComponent*> ArmorMeshes;

	UPROPERTY(Transient)
	AReplicatedStaticMeshActor* RightHandWeaponActor;

	UPROPERTY(Transient)
	AReplicatedStaticMeshActor* LeftHandWeaponActor;

	UPROPERTY(Replicated, Transient)
	int NumberOfBasicAttacks;

	UPROPERTY(Transient, BlueprintReadWrite)
	bool bCanProgressCombo;

	UPROPERTY(Transient)
	bool bEndMeleeNormally;

	UPROPERTY(Transient, BlueprintReadWrite)
	bool bCanCancelAbility;

	UPROPERTY(Transient)
	int ComboProgress;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	UWidgetComponent* FloatingNameComponent;
	

protected:
	UFUNCTION()
	void OnRep_RefreshSpellsPanelWidget();

	UPROPERTY(Transient)
	FPlayerOptions Options;

public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION(BlueprintCallable)
	FPlayerOptions& GetPlayerOptions() noexcept;

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void CallInventory();

	// Action handlers
	void HandleAttackComboActionPressed();
	void HandleAttackComboActionReleased();

	void HandleConfirmActionPressed();
	void HandleCancelActionPressed();

	CREATE_HANDLERS_FOR_ABILITY(JumpAction);
	CREATE_HANDLERS_FOR_ABILITY(DodgeAction);
	CREATE_HANDLERS_FOR_ABILITY(PotionAction);
	CREATE_HANDLERS_FOR_ABILITY(RightClickAction);
	CREATE_HANDLERS_FOR_ABILITY(Skill1Action);
	CREATE_HANDLERS_FOR_ABILITY(Skill2Action);
			
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SendLocalInputToASC(const bool bIsPressed, const EGASAbilityInputID AbilityInputID) const;
	void SendLocalInputToASC(const bool bIsPressed, const int32 AbilityInputID) const;

	void ResetLastInputVector();

	UFUNCTION(Server, Reliable)
	void ServerRPCLastInputVector(const FVector2D InputVector);

	UFUNCTION()
	void ProgressCombo();

public:
	UFUNCTION(Server,Reliable,BlueprintCallable)
	void InstantDeath();

	UFUNCTION(BlueprintCallable)
	void ResetCombo();

	UFUNCTION()
	bool GetEndMeleeNormally() const;

	/** Returns CameraBoom sub-object **/
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera sub-object **/
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** Returns Pick-Up Component sub-object **/
	FORCEINLINE UPickUpComponent* GetPickUpComponent() const noexcept { return PickUpComponent; }
	/** Returns Pick-Up Component sub-object **/
	FORCEINLINE UInteractionComponent* GetInteractionComponent() const noexcept { return InteractionComponent; }
	/** Returns Players Stats Measuring Component sub-object **/
	FORCEINLINE UPlayersStatsMeasuringComponent* GetPlayersStatsMeasuringComponent() const noexcept { return PlayersStatsMeasuringComponent; }
	/** Returns the Setup Player Input sub-object **/
	FORCEINLINE FSetupPlayerInput& GetOnSetupPlayerInput() noexcept { return OnSetupPlayerInput; }

#pragma region Inventory

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FItemInfo GetWeaponKitInfo() const noexcept { return WeaponKitInfo; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FItemInfo GetArmorKitInfo() const noexcept { return ArmorKitInfo; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FItemInfo GetGlovesInfo() const noexcept { return GlovesInfo; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FItemInfo GetBeltInfo() const noexcept { return BeltInfo; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FItemInfo GetRingInfo() const noexcept { return RingInfo; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FItemInfo GetNecklaceInfo() const noexcept { return NecklaceInfo; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FItemInfo GetEarringsInfo() const noexcept { return EarringsInfo; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool HasWeaponKit() const noexcept { return bHasWeaponKit; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool HasArmorKit() const noexcept { return bHasArmorKit; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool HasGloves() const noexcept { return bHasGloves; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool HasBelt() const noexcept { return bHasBelt; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool HasRing() const noexcept { return bHasRing; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool HasNecklace() const noexcept { return bHasNecklace; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool HasEarrings() const noexcept { return bHasEarrings; }

	UFUNCTION()
	void UnequipArmorKit(bool bIsFirstInit = false);
	UFUNCTION()
	void UnequipWeaponKit(const bool bWillEquipAfter);
	UFUNCTION()
	void UnequipGloves();
	UFUNCTION()
	void UnequipBelt();
	UFUNCTION()
	void UnequipRing();
	UFUNCTION()
	void UnequipNecklace();
	UFUNCTION()
	void UnequipEarrings();

	UFUNCTION()
	void EquipArmorKit(const FItemInfo& ArmorKit);
	UFUNCTION()
	void EquipWeaponKit(const FItemInfo& WeaponKit);
	UFUNCTION()
	void EquipGloves(const FItemInfo& Gloves);
	UFUNCTION()
	void EquipBelt(const FItemInfo& Belt);
	UFUNCTION()
	void EquipRing(const FItemInfo& Ring);
	UFUNCTION()
	void EquipNecklace(const FItemInfo& Necklace);
	UFUNCTION()
	void EquipEarrings(const FItemInfo& Earrings);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_EquipKitToPlayer(APlayerCharacter* Character, FItemInfo ItemInfo);
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_UnequipKitFromPlayer(APlayerCharacter* Character, FItemInfo ItemInfo);

#pragma endregion

	UFUNCTION()
	void SetComboGivenAbilitiesFromBuff(TArray<TSubclassOf<UEfhorisGameplayAbility>> _BuffGivenAbilities);

	UFUNCTION()
	void RemoveComboGivenAbilitiesFromBuff();

	UFUNCTION()
	FVector2D GetLastInputVector() const noexcept;

	UFUNCTION()
	void SpawnArmorMeshes(FName RowName);

	UFUNCTION()
	void RemoveArmorMeshes();

	UFUNCTION()
	void ResetArmorMeshes();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnArmorMeshes(FName RowName);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_RemoveArmorMeshes();

	UFUNCTION()
	void ToggleRightWeaponVisibility() const;

	UFUNCTION()
	void ToggleLeftWeaponVisibility() const;

	UFUNCTION()
	void SetLeftWeaponVisibility(const bool bIsHidden) const;

	UFUNCTION(BlueprintCallable)
	AEfhorisGameState* GetGameState() const;

	virtual void Die() override;

	UFUNCTION()
	float GetPotionCharge() const;

	UFUNCTION()
	float GetMaxPotionCharge() const;

	UFUNCTION()
	void Regen();

	UFUNCTION()
	void SetPotionCharge(int Charges);

protected:
	FActiveGameplayEffectHandle AddEquipmentStats(const FItemStats ItemStats) const;

#pragma region Interaction
protected:
	UPROPERTY(Transient)
	int NbInteraction;

	UPROPERTY(EditAnywhere)
	FText InteractionText;

	UPROPERTY(Replicated, Transient)
	bool bIsInteracting;

	UPROPERTY(Replicated, Transient, BlueprintReadOnly)
	bool bIsTrading;

	UPROPERTY(Replicated, Transient, BlueprintReadOnly)
	bool bIsSmithing;

	UPROPERTY(Replicated, Transient, BlueprintReadOnly)
	FItemInfo SmithItem;

	UPROPERTY(Replicated, Transient, BlueprintReadOnly, ReplicatedUsing=OnRep_HasSmithItem)
	bool bHasSmithItem;

	UFUNCTION()
	void OnRep_HasSmithItem();

	UPROPERTY(Transient)
	APlayerCharacter* PlayerHealingMe;

public:
	virtual bool IsInteractible(const APlayerCharacter* Player) const override;
	virtual void StartInteract(APlayerCharacter* Player) override;
	virtual void StopInteract(APlayerCharacter* Player) override;
	virtual const FText GetInteractionText() const override;

	UFUNCTION(Server, Reliable)
	void Server_AddInteraction();

	UFUNCTION(Server, Reliable)
	void Server_RemoveInteraction();

	UFUNCTION(BlueprintCallable)
	void StartMercenaryReviveMe(AMercenary* Mercenary);
	UFUNCTION(BlueprintCallable)
	void StopMercenaryReviveMe(AMercenary* Mercenary);

	/** Returns the interact action **/
	FORCEINLINE UInputAction* GetInteractAction() const noexcept { return InteractAction; }

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetAllInteractionBooleans(bool IsInteracting, bool IsTrading, bool IsSmithing);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetIsInteracting(bool IsInteracting);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetIsTrading(bool IsTrading);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetIsSmithing(bool IsSmithing);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsInteracting() const noexcept { return bIsInteracting; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsSmithing() const noexcept { return bIsSmithing; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool HasSmithItem() const noexcept { return bHasSmithItem; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetHasSmithItem(bool bHasSmithItem_) { bHasSmithItem = bHasSmithItem_; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetSmithItem(FItemInfo SmithItem_) { SmithItem = SmithItem_; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FItemInfo GetSmithItem() const noexcept { return SmithItem; }

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SendReviveEvent();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_TryBuyItem(AMerchant* Merchant, FItemInfo ItemInfo);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_TrySellItem(AMerchant* Merchant, FItemInfo ItemInfo);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_TrySetSmithItem(FItemInfo ItemInfo);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_TryRemoveSmithItem();
#pragma endregion

#pragma region Mercenary
private:
	UPROPERTY(Transient)
	TArray<AMercenary*> Mercenaries;

public:
	UFUNCTION()
	TArray<AMercenary*>& GetMercenaries();

	UFUNCTION()
	void AddMercenary(AMercenary* Mercenary);

	UFUNCTION()
	void RemoveMercenary(AMercenary* Mercenary);

	UFUNCTION()
	void MercenariesFindNewMaster();

#pragma endregion
};