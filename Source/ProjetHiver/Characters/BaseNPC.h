#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "Containers/Map.h"
#include "ProjetHiver/AI/AIAbility.h"
#include "BaseNPC.generated.h"

class UFloatingHealthBar;
class UWidgetComponent;

UCLASS()
class PROJETHIVER_API ABaseNPC : public ABaseCharacter
{
	GENERATED_BODY()

protected:
	UFUNCTION()
	void MoveTo(const FVector& Location, const float AcceptanceRadius = 0.0f) const;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void Die() override;
	virtual void ClearUnitReferences();

public:
	ABaseNPC(const FObjectInitializer& ObjectInitializer);
	virtual void K2_DestroyActor() override;

	UFUNCTION()
	void SetHealthBarVisibility(const bool bIsVisible) const;

	UFUNCTION(BlueprintCallable)
	void MoveToCombatRange() const;

	UFUNCTION(BlueprintCallable)
	float GetAttackRange() const noexcept { return AttackRange; }

	UFUNCTION(BlueprintCallable)
	float GetCombatRange() const noexcept { return CombatRange; }

	UFUNCTION(BlueprintCallable)
	float GetVisionRange() const noexcept { return VisionRange; }
	
protected:
	UPROPERTY(Transient)
	TArray<AActor*> ActorsToDestroyOnDeath;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI")
	TSubclassOf<UFloatingHealthBar> FloatingHealthBarClass;

	UPROPERTY()
	UFloatingHealthBar* FloatingHealthBar;

	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* FloatingHealthBarComponent;

	FDelegateHandle HealthChangedDelegateHandle;

	virtual void HealthChanged(const FOnAttributeChangeData& Data);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DieTimer = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttackRange = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CombatRange = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float VisionRange = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float VisionAngle = 45.0f;

#pragma region Threat
protected:
	UPROPERTY(Transient)
	TWeakObjectPtr<class UPawnSensingComponent> SensingComponent;

	UPROPERTY(Transient)
	TMap<ABaseCharacter*, int> ThreatMap;

	UPROPERTY(Transient,VisibleAnywhere)
	ABaseCharacter* Target = nullptr;

	UPROPERTY()
	FString TargetId;

	TPair<ABaseCharacter*,int> MaxThreat();

	UFUNCTION()
	void EvaluateNewTarget();

public:
	UFUNCTION()
	void AddThreat(ABaseCharacter* Target_, const int& ThreatAmount, const bool bPropagate = true);

	UFUNCTION(BlueprintCallable)
	void ClearThreat(ABaseCharacter* Target_);

	UFUNCTION()
	void Taunt(ABaseCharacter* Source, const float TauntStrength);

	UFUNCTION(BlueprintCallable)
	bool HasTarget() const noexcept { return IsValid(Target); }

	UFUNCTION(BlueprintCallable)
	ABaseCharacter* GetTarget() const noexcept { return Target; }

	void SetTargetId(const FString& Id) noexcept { TargetId = Id; }

	const FString& GetTargetId() const noexcept { return TargetId; }

	bool HasTargetId() const noexcept { return !TargetId.IsEmpty(); }

	UFUNCTION(BlueprintCallable)
	void OnSeePawn(APawn* OtherPawn);

	UFUNCTION()
	void Spot(ABaseCharacter* Target_);

	TMap<ABaseCharacter*, int>& GetThreatMap();

	UFUNCTION()
	void DrawDebugThreat(const float& DeltaSeconds);

	FVector GetInitialLocation() const noexcept { return InitialLocation; }

	void SetInitialLocation(FVector InitialLocation_) noexcept { InitialLocation = InitialLocation_; }

	UFUNCTION()
	void SetTarget(ABaseCharacter* Target_);

	void SetThreatMap(TMap<ABaseCharacter*, int> ThreatMap_) noexcept { ThreatMap = ThreatMap_; }

#pragma endregion

#pragma region Patrol
protected:
	UPROPERTY(Transient, BlueprintReadOnly)
	FVector InitialLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PatrolRadius = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PatrolSpeed = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PatrolIdleMinTimeInSeconds = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PatrolIdleMaxTimeInSeconds = 5.f;
#pragma endregion

#pragma region Combat
protected:
	UPROPERTY(Transient,Replicated, BlueprintReadWrite)
	bool bIsInCombat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CombatSpeed = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FAIAbility> CombatCombos;

	UPROPERTY(Transient, BlueprintReadOnly)
	FAIAbility ChosenCombo;

	UFUNCTION(BlueprintCallable)
	void ChooseCombo();

	UFUNCTION()
	void InitializeNPCAbilities();

	UFUNCTION(BlueprintImplementableEvent)
	void EnterCombat();

public:
	UFUNCTION(BlueprintImplementableEvent)
	void ExitCombat();
#pragma endregion

#pragma region Block
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlockingSpeed = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxRangeDistance = 1000.f;

	UPROPERTY(Transient, BlueprintReadWrite)
	bool bIsStrafingLeft;
#pragma endregion
};
