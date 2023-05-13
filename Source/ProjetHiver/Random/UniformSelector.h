#pragma once

#include "CoreMinimal.h"

/**
 * Random Selector with following Uniform Law
 */
template <class T>
class FUniformSelector
{
	/** Array of elements pickable. */
	TArray<T> Elements;

	/** Number of elements pickable. */
	size_t Count;

public:
	/**
	 * Pop a random element from the array.
	 * No verification are done, be sure the array isn't empty.
	 * 
	 * @return the element poped
	 */
	T Pop() {
		size_t Index = RandomIndex();
		T Element = Elements[Index];
		Discard(Index);
		return Element;
	}

	/**
	 * Pop Nb random element from the array.
	 * No verification are done, be sure the array isn't empty.
	 *
	 * @param Nb number of elements to pop
	 * @return the elements poped
	 */
	TArray<T> Pop(size_t Nb) {
		TArray<T> Elements_;
		Elements_.Reserve(Nb);
		for (size_t i = 0; i < Nb; ++i) {
			size_t Index = RandomIndex();
			Elements_.Add(Elements[Index]);
			Discard(Index);
		}

		return Elements_;
	}

	/**
	 * Reset poped elements into the array of available elements.
	 * 
	 */
	void Reset() noexcept {
		Count = Elements.Num();
	}

	/**
	 * Reset the available elements.
	 *
	 * @param Elements_ the new array of elements
	 */
	void Reset(TArray<T> Elements_) noexcept {
		Elements = Elements_;
		Reset();
	}

private:
	/** Return a random index following uniform law. */
	size_t RandomIndex() const noexcept {
		return FMath::RandRange(0, Count - 1);
	}

	/** Discard the element at index from the available elements. */
	void Discard(size_t Index) noexcept {
		Elements.Swap(Index, Count - 1);
		--Count;
	}
};
