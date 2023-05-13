#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "ProjetHiver/Widgets/DPSWidget.h"
#include "Dummy.generated.h"

USTRUCT()
struct FTimedDamage
{
	GENERATED_BODY()

		UPROPERTY(Transient)
		float Time = 0.0f;

	UPROPERTY(Transient)
		float Damage = 0.0f;

	bool operator==(const FTimedDamage& other) const
	{
		return Time == other.Time && Damage == other.Damage;
	}

};

UCLASS()
class PROJETHIVER_API ADummy : public AEnemy
{
	GENERATED_BODY()


protected:
	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* DPSWidgetComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI")
	TSubclassOf<UDPSWidget> DPSWidgetClass;

	UPROPERTY(Transient)
	UDPSWidget* DPSWidget;

	UPROPERTY(Transient, BlueprintReadOnly)
	float DPS = 0.0f;

	UPROPERTY(EditAnywhere)
	float TimeInterval = 10.0f;

	UPROPERTY(Transient)
	TArray<FTimedDamage> DamageTaken;

	virtual void Tick(float DeltaSeconds) override;

	virtual void BeginPlay() override;

	UFUNCTION()
	void IncrementTime(const float DeltaTime);

public:
	ADummy(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(NetMulticast,Reliable)
	void AddDamage(const float Damage);
};
