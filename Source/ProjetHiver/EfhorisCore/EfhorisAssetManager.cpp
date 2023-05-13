#include "EfhorisAssetManager.h"
#include "AbilitySystemGlobals.h"

void UEfhorisAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	UAbilitySystemGlobals::Get().InitGlobalData();
}
