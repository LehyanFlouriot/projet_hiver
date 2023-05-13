#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EfhorisPlayerController.generated.h"

class AMerchant;
class ABaseCharacter;
class AContract;
class UAkAudioEvent;
class APlayerCharacter;
class UDamageNumber;
class UGameplayEffect;
class UFloatingHealthBar;
class ULevelSequence;
class ULevelSequencePlayer;
enum class ESpellSlotType : uint8;

UCLASS()
class PROJETHIVER_API AEfhorisPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient, Replicated, ReplicatedUsing=OnRep_MusicTargets)
	TArray<ABaseCharacter*> MusicTargets;

	UFUNCTION()
	void OnRep_MusicTargets();

protected:

	UPROPERTY(EditAnywhere,NoClear)
	TSubclassOf<UDamageNumber> DamageNumberClass;

	UPROPERTY(EditAnywhere, NoClear)
	TSubclassOf<UCameraShakeBase> OnHitCameraShake;

	UPROPERTY(EditAnywhere, NoClear)
	TSubclassOf<UGameplayEffect> CheatBuffEffect;

	UPROPERTY(BlueprintReadOnly,Transient)
	int NbOpenedMenus;

	UPROPERTY(Replicated,Transient)
	int MenuCooldown;

	UPROPERTY()
	bool bChestOpened = false;

	UPROPERTY()
	bool bIsShopPanelOpened = false;

	UPROPERTY()
	bool bIsContractPanelOpened = false;

	UPROPERTY()
	bool bIsMedicPanelOpened = false;

	UPROPERTY()
	bool bIsSmithPanelOpened = false;

	UPROPERTY()
	bool bHasLeftSafeZone = false;
	UPROPERTY()
	bool bHasPlaySafeZoneSequence = false;

	UPROPERTY(BlueprintReadWrite)
	bool bIsMinimapOpened;

	UPROPERTY(Transient, BlueprintReadWrite)
	ULevelSequencePlayer* LevelSequencePlayer = nullptr;

	UPROPERTY(EditAnywhere)
	UAkAudioEvent* OpenShopPanelEvent;

	UPROPERTY(EditAnywhere)
	UAkAudioEvent* OpenContractPanelEvent;

	UPROPERTY(EditAnywhere)
	UAkAudioEvent* OpenMedicPanelEvent;

	UPROPERTY(EditAnywhere)
	UAkAudioEvent* OpenSmithPanelEvent;

	UPROPERTY(EditAnywhere)
	UAkAudioEvent* ClosePanelEvent;

	UPROPERTY(EditAnywhere)
	UAkAudioEvent* OpenMinimapEvent;

	UPROPERTY(EditAnywhere)
	UAkAudioEvent* CallInventorySound;

	UPROPERTY(EditAnywhere)
	UAkAudioEvent* PlayerLoseEvent;

public:
	UPROPERTY()
	bool bIsMainMenuOpened;

	friend class UEfhorisCheatManager;

	AEfhorisPlayerController();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION()
	void AddOpenMenu();

	UFUNCTION()
	void RemoveOpenMenu();

	virtual void AcknowledgePossession(APawn* P) override;

	virtual void ClientRestart_Implementation(class APawn* NewPawn) override;

	UFUNCTION(Server, Reliable)
	void Server_IncrementMenuCooldown(const bool bAddCooldown);

	UFUNCTION(Server, Reliable)
	void Server_ResetMenuCooldown();

	UFUNCTION()
	bool IsInMenuCooldown() const noexcept { return MenuCooldown > 0;}

	UFUNCTION(Client, Reliable)
	void Client_RefreshOpenedPanels();

	UFUNCTION(Client, Reliable)
	void Client_RefreshSpellsPanel();

	UFUNCTION(Server, Reliable)
	void Server_CallInventoryRequest();

	UFUNCTION(Client, Reliable)
	void Client_OpenInventoryPanel();

	UFUNCTION(Client, Reliable)
	void Client_CloseInventoryPanel();

	UFUNCTION()
	void OpenInventoryPanel();

	UFUNCTION()
	void CloseInventoryPanel();
    
	UFUNCTION(Client, Reliable)
	void Client_OpenShopPanel(AMerchant* Merchant);

	UFUNCTION(Client, Reliable)
	void Client_CloseShopPanel();

	UFUNCTION()
	void OpenShopPanel();

	UFUNCTION()
	void CloseShopPanel();

	UFUNCTION(Client, Reliable)
	void Client_OpenMedicPanel();

	UFUNCTION(Client, Reliable)
	void Client_CloseMedicPanel();

	UFUNCTION()
	void OpenMedicPanel();

	UFUNCTION()
	void CloseMedicPanel();

	UFUNCTION(Client, Reliable)
	void Client_OpenSmithPanel();

	UFUNCTION(Client, Reliable)
	void Client_CloseSmithPanel();

	UFUNCTION()
	void OpenSmithPanel();

	UFUNCTION()
	void CloseSmithPanel();

	UFUNCTION(Client, Reliable)
	void Client_OpenContractPanels();

	UFUNCTION(Client, Reliable)
	void Client_CloseContractPanels();

	UFUNCTION()
	void OpenContractPanels();

	UFUNCTION()
	void CloseContractPanels();

	UFUNCTION(Client, Reliable)
	void Client_ShowDamageNumber(ABaseCharacter* Target, const float DamageAmount) const;

	UFUNCTION(Client, Unreliable)
	void Client_OnHitShake(const float Scale = 1.0f) const;

	UFUNCTION()
	void ShowInteractPrompt() const;

	UFUNCTION()
	void HideInteractPrompt() const;

	UFUNCTION()
	void Authority_TeleportPlayer(const FVector& Vector);

	UFUNCTION(Client, Reliable)
	void Client_ShowFloatingHealthBar(const ABaseNPC* NPC, const bool bIsVisible);

	UFUNCTION(Client, Reliable)
	void Client_ResetHealthPotion() const;

	UFUNCTION(Client, Reliable)
	void Client_UpdatePlayer() const;

	UFUNCTION()
	void Authority_UpdateHealthBars() const;

	UFUNCTION()
	void Authority_UpdateMapNicknames() const;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_AcceptContract(AContract* Contract);

	UFUNCTION(Client, Reliable)
	void Client_RefreshSideContracts();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_TryHealAllPlayers();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_TrySmithItem();

	UFUNCTION(Client, Reliable)
	void Client_CooldownSpell(float Cooldown, ESpellSlotType SpellType);

	UFUNCTION(Client, Reliable)
	void Client_InitCooldownSpell(ESpellSlotType SpellType);

	UFUNCTION(Client, Reliable)
	void Client_CancelCooldownSpell(ESpellSlotType SpellType);

	UFUNCTION()
	void ShowCreatorOptions() const;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetAutoSave(bool bIsActivated) const;

	UFUNCTION(BlueprintCallable)
	void SaveExit() const;

	UFUNCTION(Server, Reliable)
	void Server_SaveGame() const;

	UFUNCTION(Client, Reliable)
	void Client_QuitGame() const;

	UFUNCTION()
	void QuitGame(const bool bShouldSave = true) const;

	UFUNCTION(Client, Reliable)
	void Client_Win();

	UFUNCTION(Client, Reliable)
	void Client_Lose();

	UFUNCTION()
	void GenerateMinimap(const TArray<FLocatedTile>& Layout);

	UFUNCTION(Client,Reliable)
	void Client_ForceCloseAllMenus(const bool bPlayAnimation = false);

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_ShowCrosshair() const;

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_HideCrosshair() const;

	UFUNCTION(Client, Reliable)
	void Client_ShowChestMoving() const;

	UFUNCTION(Client, Reliable)
	void Client_HideChestMoving() const;

	UFUNCTION(Client, Reliable)
	void Client_PlaySequence(ULevelSequence* SequenceToPlay, bool bShouldPlayStartSequenceAfter = false);

	UFUNCTION()
	void PlaySafeZoneSequence();

	UFUNCTION()
	void PlayEndSequence();

	UFUNCTION(BlueprintCallable)
	void SkipSequence();

	UFUNCTION()
	void PlayCallInventorySound();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_PlayEvent(UAkAudioEvent* MusicToPlay);

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_LeftSafeZone();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_EnterSafeZone();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_StopAllMusic();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_SetMusicState(FName StateGroup, FName NewState);
};
