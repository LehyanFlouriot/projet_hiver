#include "Wolf.h"

#include "ProjetHiver/AI/Group.h"

void AWolf::ClearUnitReferences()
{
	if (IsValid(Group)) {
		Group->RemoveUnit(this);
	}
	Super::ClearUnitReferences();
}

void AWolf::SetGroup(AGroup* Group_) {
	Group = Group_;
}
