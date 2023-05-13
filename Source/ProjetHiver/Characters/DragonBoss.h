#pragma once

#include "CoreMinimal.h"
#include "ProjetHiver/Characters/BaseCharacter.h"
#include "DragonBoss.generated.h"

class UHealthBoss;
class UWidgetComponent;

UCLASS()
class PROJETHIVER_API ADragonBoss : public ABaseCharacter
{
	GENERATED_BODY()

public:
	void ClearUnitReferences();
	virtual void K2_DestroyActor() override;

	ADragonBoss(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

public:
	virtual void Die() override;

	UFUNCTION(Server, Reliable)
	void ServerWinGameRequest();

	UFUNCTION()
	void SetIsFlying(bool bFly) noexcept { bIsFlying = bFly; }

	UFUNCTION()
	bool GetIsFlying() const noexcept { return bIsFlying; }

	UFUNCTION()
	void SetIsInCombat(bool bCombat) noexcept { bIsInCombat = bCombat; }

	UFUNCTION()
	bool GetIsInCombat() const noexcept { return bIsInCombat; }

	UFUNCTION(BlueprintCallable)
	float GetFlyOffset() const noexcept { return FlyOffset; }

	UFUNCTION(BlueprintCallable)
	FVector GetInitialLocation() const noexcept { return InitialLocation; }

	UFUNCTION()
	void SetInitialLocation(FVector InitialLocation_) noexcept { InitialLocation = InitialLocation_; }

	UFUNCTION(BlueprintCallable)
	int GetPhase() const noexcept { return Phase; }

	UFUNCTION()
	void SetPhase(int Phase_) noexcept { Phase = Phase_; }

	UFUNCTION(BlueprintCallable)
	void IncreasePhase();

	UFUNCTION(NetMulticast, Reliable)
	void HideHealthBar();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void ShowHealthBar();

	UFUNCTION()
	void ScaleBoss(const int NbPlayers) const noexcept;

protected:
	UPROPERTY(Transient)
	UHealthBoss* HealthBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DieTimer = 5.f;

	UPROPERTY(Transient, Replicated, BlueprintReadWrite)
	float RotationSpeed;

	FDelegateHandle HealthChangedDelegateHandle;

	UPROPERTY(Replicated, Transient, BlueprintReadWrite)
	bool bIsFlying = false;

	UPROPERTY(Replicated, Transient, BlueprintReadWrite)
	bool bIsInCombat = false;

	UPROPERTY(EditAnywhere)
	float FlyOffset = 600.f;

	UPROPERTY(Transient, BlueprintReadOnly)
	FVector InitialLocation;

	virtual void HealthChanged(const FOnAttributeChangeData& Data);

	UFUNCTION(NetMulticast, Unreliable, BlueprintCallable)
	void UpdateUIHealthBar();

	UPROPERTY(Replicated, Transient, BlueprintReadWrite)
	int Phase = 1;

	UPROPERTY()
	float InitialMaxHealth;

#pragma region Detection

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PlayerDetectionRadius = 6000;

#pragma endregion

#pragma region RangeParam

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CloseRangeRadius = 1000;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MidRangeRadius = 2000;

#pragma endregion
};
