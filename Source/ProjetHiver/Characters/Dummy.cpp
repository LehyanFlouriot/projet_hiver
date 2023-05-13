#include "Dummy.h"
#include "Components/WidgetComponent.h"

void ADummy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!HasAuthority())
	{
		IncrementTime(DeltaSeconds);

		if (DPSWidget)
		{
			DPSWidget->SetDPS(DPS);
			//My widget won't work so I'm using this ugly print
			GEngine->AddOnScreenDebugMessage(-1, 50.f, FColor::Blue, "DPS : " + FString::SanitizeFloat(DPS));
		}
	}
}

void ADummy::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority() && DPSWidgetClass)
	{
		DPSWidget = CreateWidget<UDPSWidget>(GetWorld()->GetFirstPlayerController(), DPSWidgetClass);
		if (DPSWidget && DPSWidgetComponent)
		{
			DPSWidgetComponent->SetWidget(DPSWidget);
		}
	}
}

void ADummy::IncrementTime(const float DeltaTime) {
	TArray<FTimedDamage> ToRemove;
	for (auto& TimedDamage : DamageTaken)
	{
		TimedDamage.Time += DeltaTime;
		if (TimedDamage.Time > TimeInterval)
		{
			ToRemove.Add(TimedDamage);
		}
	}
	for (auto& TimedDamage : ToRemove)
	{
		DamageTaken.Remove(TimedDamage);
		DPS -= TimedDamage.Damage / TimeInterval;
	}

}

ADummy::ADummy(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {

	DPSWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("DPSWidgetComponent"));
	DPSWidgetComponent->SetupAttachment(RootComponent);
}

void ADummy::AddDamage_Implementation(const float Damage) {
	if (!HasAuthority())
	{
		FTimedDamage TimedDamage = FTimedDamage();
		TimedDamage.Damage = Damage;
		DamageTaken.Add(TimedDamage);
		DPS += Damage / TimeInterval;
	}
}
