#include "EfhorisAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "ProjetHiver/Characters/BaseNPC.h"

void AEfhorisAIController::RunBehavior() {
	if (Behavior)
	{
		RunBehaviorTree(Behavior);
	}
}

void AEfhorisAIController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
{
	if (const ABaseNPC* NPC = Cast<ABaseNPC>(GetPawn()); IsValid(NPC))
		if (NPC->IsStunned()) return;

	Super::UpdateControlRotation(DeltaTime, bUpdatePawn);
}

void AEfhorisAIController::OnPossess(APawn* InPawn) {
	if (AAIController* PrevController = InPawn->GetController<AAIController>()) {
		PrevController->UnPossess();
	}
	Super::OnPossess(InPawn);
	if (InPawn && Behavior) {
		RunBehaviorTree(Behavior);
	}
}
