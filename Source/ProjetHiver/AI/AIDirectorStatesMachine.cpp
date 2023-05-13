#include "AIDirectorStatesMachine.h"

#include "AIDirector.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"

void FAIDirectorFSM::Update()
{
	// Lire etat courant
	// Tenter de realiser les transitions en fonctions des conditions de celles ci
	if(!bCheatIntensityActivated)
		UpdateState();
}

void FAIDirectorFSM::UpdateState() noexcept
{
	if (ConditionToSwitchToNewState()) {
		CurrentStateIndex = GetNextStateIndex();
		CurrentNumberOfRoomVisited = GameState->GetNbVisitedTiles();
	}
}

bool FAIDirectorFSM::ConditionToSwitchToNewState() const
{
	return GameState->GetNbVisitedTiles() - CurrentNumberOfRoomVisited >= MinRoomsWithCurrentState;
}

int FAIDirectorFSM::GetNextStateIndex() const noexcept
{
	int NextStateIndex = CurrentStateIndex + 1;
	if (NextStateIndex == States.Num()) {
		NextStateIndex = 0;
	}
	return NextStateIndex;
}

void FAIDirectorFSM::SetIntensity(int Intensity_) noexcept
{
	if (Intensity_ == -1)
	{
		bCheatIntensityActivated = false;
		UpdateState();
	}
	else
	{
		bCheatIntensityActivated = true;
		CurrentStateIndex = Intensity_;
	}
}
