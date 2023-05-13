#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "LobbyPlayerState.generated.h"

USTRUCT(BlueprintType)
struct PROJETHIVER_API FLobbyInfo {

	GENERATED_BODY();

	UPROPERTY(BlueprintReadWrite)
	FString NickName;

	UPROPERTY(BlueprintReadWrite)
	bool bIsReady = false;
};

/**
 * 
 */
UCLASS()
class PROJETHIVER_API ALobbyPlayerState : public APlayerState
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void ToggleIsReady() noexcept { bIsReadyToLaunch = !bIsReadyToLaunch; }
	void SetIsReady(bool bR) noexcept { bIsReadyToLaunch = bR; }
	bool IsReady() const noexcept { return bIsReadyToLaunch; }

	UFUNCTION(Client, Reliable)
	void Client_GetInformations();

	UFUNCTION(Server, Reliable)
	void Server_SetInformations(const FString& NickName_, const FString& UserId_);

	UFUNCTION()
	void OnRep_RefreshLobby() const;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RefreshLobby() const;

	void PlayerResfreshLobby() const;

	const FString& GetNickName() const noexcept { return SteamNickName; }

	void SetNickName(const FString& NickName_) { SteamNickName = NickName_; }

private:
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_RefreshLobby)
	bool bIsReadyToLaunch = false;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_RefreshLobby)
	FString SteamNickName;
};
