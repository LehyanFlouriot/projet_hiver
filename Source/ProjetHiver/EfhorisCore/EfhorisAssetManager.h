#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "EfhorisAssetManager.generated.h"

UCLASS()
class PROJETHIVER_API UEfhorisAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	virtual void StartInitialLoading() override;
};
