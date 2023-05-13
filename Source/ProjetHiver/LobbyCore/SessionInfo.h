#pragma once

#include "CoreMinimal.h"
#include "SessionInfo.generated.h"

USTRUCT(BlueprintType)
struct PROJETHIVER_API FSessionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString SessionName;

	UPROPERTY(BlueprintReadOnly)
	int PlayerNumber {};

	UPROPERTY(BlueprintReadOnly)
	int MaxPlayerNumber {};

	UPROPERTY(BlueprintReadOnly)
	bool bIsPrivate = false;

	UPROPERTY(BlueprintReadOnly)
	FString HostName;

	UPROPERTY(BlueprintReadOnly)
	bool bIsDedicated = false;

	bool bUpdated = false;

	bool operator==(const FSessionInfo& Other) const {
		return (SessionName == Other.SessionName) 
			&& (HostName == Other.HostName) 
			&& (bIsDedicated== Other.bIsDedicated);
	}
};
