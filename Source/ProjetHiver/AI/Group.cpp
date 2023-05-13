#include "Group.h"

#include "ProjetHiver/Characters/Wolf.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameMode.h"

void AGroup::BeginPlay()
{
	Super::BeginPlay();
	if (!WolfClass) Destroy();

	const FTransform Transform = GetActorTransform();
	const float Angle = 2 * PI / InitialUnitCount;
	const float Radius = 50 + 10 * InitialUnitCount;
	const FVector Axis = FVector(0, 0, 1);
	FVector Offset = Radius * Transform.GetRotation().GetForwardVector();
	if (AEfhorisGameMode* GameMode = GetWorld()->GetAuthGameMode<AEfhorisGameMode>(); IsValid(GameMode)) {
		for (int i = 0; i < InitialUnitCount; i++) {
			Offset = Offset.RotateAngleAxisRad(Angle, Axis);
			FTransform TransformOffset = FTransform(Transform);
			TransformOffset.AddToTranslation(Offset);
			AWolf* Unit = GameMode->SpawnActorDeferred<AWolf>(WolfClass, TransformOffset);
			AddUnit(Unit);
			Unit->FinishSpawning(TransformOffset);
		}
	}
}

void AGroup::SetLevel(const int Level_) {
	Level = Level_;
}

void AGroup::SetEnemyCount(const int Count) {
	InitialUnitCount = Count;
}

void AGroup::AddUnit(AWolf* Unit) {
	if (IsValid(Unit)) {
		Units.Add(Unit);
		Unit->SetCharacterLevel(Level);
		Unit->SetGroup(this);
	}
}

void AGroup::RemoveUnit(AWolf* Unit) {
	if (Units.RemoveSingle(Unit)) {
		if (Units.IsEmpty()) {
			Destroy(); //May cause issues when K2_Destroying the actor
		}
	}
}

TArray<AWolf*>& AGroup::GetUnits() {
	return Units;
}

void AGroup::K2_DestroyActor() {
	while (!Units.IsEmpty())
	{
		if (AWolf* Wolf = Units[0]; IsValid(Wolf)) {
			Wolf->K2_DestroyActor();
		}
		else {
			//May produce stray wolves that are not cleaned up by AI Director (it's a *feature*)
			Units.Remove(Wolf);
		}
	}
	Super::K2_DestroyActor();
}