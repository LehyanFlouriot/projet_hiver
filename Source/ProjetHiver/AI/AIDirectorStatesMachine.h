#pragma once

#include "CoreMinimal.h"
class AEfhorisGameState;

/**
 * Efhoris implementation of AI Director Finite State Machine.
 */
class PROJETHIVER_API FAIDirectorFSM
{
	/** Array representing every possible states. */
	TArray<FName> States;

	/** Index of current state. */
	int CurrentStateIndex = 0;

public:
	FAIDirectorFSM() = default;
	~FAIDirectorFSM() = default;

	/** 
	 * Computes the current state.
	 * Depends on Game State and Player States.
	 */
	void Update();

	/**
	 * Return the current state of the AI Director FSM.
	 *
	 * @return current state.
	 */
	FName GetCurrentStateName() const noexcept { return States[CurrentStateIndex]; }

	/**
	 * Return the current state of the AI Director FSM.
	 * Call `Update` method.
	 *
	 * @return current state.
	 */
	FName CurrentStateUpdate() {
		Update();
		return GetCurrentStateName();
	}

	inline void SetGameState(AEfhorisGameState* GameState_) noexcept { GameState = GameState_; }

	void SetStates(TArray<FName> States_) noexcept { States = States_; }

	/** Restart from first state. */
	void ResetState() noexcept { CurrentStateIndex = 0; }

	void SetIntensity(int Intensity_) noexcept;

	int GetIntensity() const noexcept { return CurrentStateIndex; }

	uint8 GetCurrentNumberOfRoomVisited() const noexcept { return CurrentNumberOfRoomVisited; }

	void SetNumberOfRoomsVisited(uint8 NbOfRoomsVisited) noexcept { CurrentNumberOfRoomVisited = NbOfRoomsVisited; }

private:
	AEfhorisGameState* GameState = nullptr;
	uint8 CurrentNumberOfRoomVisited = 1;
	uint8 MinRoomsWithCurrentState = 2;

	bool bCheatIntensityActivated = false;

	void UpdateState() noexcept;
	bool ConditionToSwitchToNewState() const;
	int GetNextStateIndex() const noexcept;
};
