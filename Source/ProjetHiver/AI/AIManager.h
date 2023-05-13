#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AIManager.generated.h"

class AEnemy;
class ADragonBoss;
class AMercenary;
class AKevin;
class UInfluenceMapManager;

UCLASS()
class PROJETHIVER_API AAIManager : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)
	TArray<AEnemy*> Enemies;

	UPROPERTY(Transient)
	TArray<ADragonBoss*> Bosses;

	UPROPERTY(Transient)
	TArray<AMercenary*> Mercenaries;

	UPROPERTY(Transient)
	AKevin* Kevin;

	UPROPERTY(EditAnywhere)
	bool bDebugEnemiesThreat = false;

	UPROPERTY(EditAnywhere)
	bool bDebugMercenariesThreat = false;

	UPROPERTY(EditAnywhere)
	bool bDebugAllIMaps = false;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Sets default values for this actor's properties
	AAIManager();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void ClearUnits();

	UFUNCTION()
	void AddUnit(ABaseCharacter* Unit);

	UFUNCTION()
	void RemoveUnit(ABaseCharacter* Unit);

	UFUNCTION()
	TArray<AEnemy*>& GetEnemies();

	UFUNCTION()
	void KillEnemies();

	UFUNCTION()
	void ClearEnemiesThreat(ABaseCharacter* Character) const;

	UFUNCTION()
	TArray<ADragonBoss*>& GetBosses();

	UFUNCTION()
	void KillBosses();

	UFUNCTION()
	TArray<AMercenary*>& GetMercenaries();

	UFUNCTION()
	void KillMercenaries();

	UFUNCTION()
	void ClearMercenariesThreat(ABaseCharacter* Character) const;

	UFUNCTION()
	void UpdateBossesScaling(const int NbPlayers) noexcept;

	UFUNCTION()
	void DrawDebug(float DeltaTime) const;

	UFUNCTION()
	UInfluenceMapManager* GetIMapManager() const;

	UFUNCTION()
	void AddKevin(AKevin* Kev) noexcept { Kevin = Kev; }

	UFUNCTION()
	void IsTargeted(const AEfhorisPlayerState* Player);

//Influence Maps
private:
	UPROPERTY(EditAnywhere, NoClear)
	TSubclassOf<UInfluenceMapManager> IMapManagerClass;

	UPROPERTY(Transient)
	UInfluenceMapManager* InfluenceMapManager = nullptr;

	UFUNCTION()
	void AddUnitsInfluence() const;
};
