#include "WeightedRandomSelector.h"

#include "WeightedActor.h"


TSubclassOf<AActor> FWeightedRandomSelector::Select(float Difficulty) const noexcept {
	CurWeightSum = WeightSum(Difficulty);
	AWeightedActor* SelectedActor = WeightedActors[WeightedRandomIndex(Difficulty)].GetDefaultObject();
	return SelectedActor->ActorClass;
}

void FWeightedRandomSelector::Reset(TArray<TSubclassOf<AWeightedActor>> WeightedActors_) noexcept {
	WeightedActors = WeightedActors_;
}

int FWeightedRandomSelector::WeightedRandomIndex(float Difficulty) const noexcept
{
	float RandomWeight_ = RandomWeight(Difficulty);

	for (int i = 0; i < WeightedActors.Num(); ++i) {
		AWeightedActor* SelectedActor = WeightedActors[i].GetDefaultObject();
		RandomWeight_ -= SelectedActor->WeightFct(Difficulty);
		if (RandomWeight_ <= 0.001)
			return i;
	}

	UE_LOG(LogTemp, Error, TEXT("The Random Selector has not found an element."))
	return 0;
}

float FWeightedRandomSelector::RandomWeight(float Difficulty) const noexcept
{
	return FMath::RandRange(0.f, CurWeightSum);
}

float FWeightedRandomSelector::WeightSum(float Difficulty) const noexcept
{
	float Sum = 0.f;
	for (int i = 0; i < WeightedActors.Num(); ++i) {
		AWeightedActor* SelectedActor = WeightedActors[i].GetDefaultObject();
		Sum += SelectedActor->WeightFct(Difficulty);
	}

	return Sum;
}
