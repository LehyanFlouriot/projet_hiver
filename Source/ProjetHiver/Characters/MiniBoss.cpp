#include "MiniBoss.h"

#include "ProjetHiver/AI/AIManager.h"
#include "ProjetHiver/Contracts/ContractStatus.h"
#include "ProjetHiver/Contracts/ExplorationContract.h"
#include "ProjetHiver/Contracts/ExplorationContractType.h"
#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "ProjetHiver/GAS/EfhorisAbilitySystemComponent.h"

void AMiniBoss::BeginPlay()
{
	Super::BeginPlay();

	if (!IsValid(AbilitySystemComponent)) return;

	if (!MiniBossModifier)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Missing MiniBossModifier for %s. Please fill in the character's Blueprint."), *FString(__FUNCTION__), *GetName());
		return;
	}

	// Can run on Server and Client
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	const FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(MiniBossModifier, CharacterLevel, EffectContext);
	if (NewHandle.IsValid())
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
}

void AMiniBoss::Die()
{
	Super::Die();

	if (const AEfhorisGameState* GameState = GetWorld()->GetGameState<AEfhorisGameState>(); IsValid(GameState))
	{
		AExplorationContract* Contract = GameState->GetExplorationContract();

		if (Contract->GetType() == EExplorationContractType::MiniBoss && Contract->GetStatus() == EContractStatus::OnGoing)
			Contract->SetStatus(EContractStatus::Succeeded);
	}
}
