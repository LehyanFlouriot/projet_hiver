#include "EGA_BlackHole.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "ProjetHiver/Characters/BaseCharacter.h"
#include "ProjetHiver/Characters/BaseNPC.h"

void UEGA_BlackHole::ApplyEffectToAll(const ABaseCharacter* Source, ABaseCharacter* Target)
{
	if (Target->IsA(ABaseNPC::StaticClass()))
	{
		FVector PullDirection = AoELocation - Target->GetActorLocation();
		PullDirection.Normalize();

		PullDirection += Target->GetActorUpVector() * 0.5f;
		PullDirection.Normalize();

		Target->LaunchCharacter(PullDirection * PullStrength, true, true);
	}
}
