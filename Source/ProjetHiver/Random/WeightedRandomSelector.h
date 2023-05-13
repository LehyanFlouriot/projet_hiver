#pragma once

#include "CoreMinimal.h"

class AWeightedActor;

/**
 * Random Selector with weight function
 */
class PROJETHIVER_API FWeightedRandomSelector {
	/**
	 * Weighted Actors available.
	 */
	TArray<TSubclassOf<AWeightedActor>> WeightedActors;

	/** Sum of weight of each actors. */	
	mutable float CurWeightSum;

public:
	/**
	 * Select a Random AActor class among available ones.
	 * No verification are done, be sure the selctor isn't empty.
	 * 
	 * @param Difficulty current value of difficulty utility
	 * @return the class chosen
	 */
	TSubclassOf<AActor> Select(float Difficulty) const noexcept;

	/**
	 * Reset the available WeightedActors.
	 * 
	 * @param WeightedActors_ the new WeightedActors
	 */
	void Reset(TArray<TSubclassOf<AWeightedActor>> WeightedActors_) noexcept;

private:
	/** 
	 * Select a Random Index associated to a weighted actor.
	 * 
	 * @param Difficulty current value of difficulty utility
	 * @return the index of the weighted actor
	 */
	int WeightedRandomIndex(float Difficulty) const noexcept;

	/** Pick a Random Weight */
	float RandomWeight(float Difficulty) const noexcept;

	/** Sum of all weights. */
	float WeightSum(float Difficulty) const noexcept;
};
