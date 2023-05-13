#pragma once

#include "CoreMinimal.h"
#include "ToggleableMenu.h"
#include "Components/CanvasPanel.h"
#include "Components/RetainerBox.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Minimap.generated.h"

struct FLocatedTile;

enum class ETileDirection : uint8;

USTRUCT()
struct FMinimapTile
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FVector2D Position = FVector2D(0,0);

	UPROPERTY(Transient)
	UCanvasPanelSlot* Slot = nullptr;
};

USTRUCT()
struct FPlayerInMinimap
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	APawn* Player = nullptr;

	UPROPERTY(Transient)
	UCanvasPanelSlot* Widget = nullptr;

	UPROPERTY(Transient)
	UCanvasPanelSlot* NicknameWidget = nullptr;

	UPROPERTY(Transient)
	UUserWidget* PlayerIconWidget = nullptr;

	bool operator==(const FPlayerInMinimap& Other) const noexcept {
		return Player == Other.Player;
	}
};

USTRUCT()
struct FBoundingBox
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	int MinX = 0;

	UPROPERTY(Transient)
	int MaxX = 0;

	UPROPERTY(Transient)
	int MinY = 0;

	UPROPERTY(Transient)
	int MaxY = 0;
};

UCLASS()
class PROJETHIVER_API UMinimap : public UToggleableMenu
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)
	TArray<FMinimapTile> Tiles;

	UPROPERTY(Transient)
	TArray<FPlayerInMinimap> PlayersInMap;

	UPROPERTY(Transient)
	int SizeX;

	UPROPERTY(Transient)
	int SizeY;

	UPROPERTY(Transient)
	float TileSize;

	UPROPERTY(EditAnywhere)
	float PlayerWidgetSize = 32.0f;

	UPROPERTY(BlueprintReadWrite, Transient)
	UCanvasPanel* TilesCanvasPanel;

	UPROPERTY(BlueprintReadWrite, Transient)
	UCanvasPanel* IconsCanvasPanel;

	UPROPERTY(BlueprintReadWrite,Transient)
	URetainerBox* RetainerBox;

	UPROPERTY(BlueprintReadWrite)
	float CanvasSize;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UTileMinimapWidget> TileMinimapWidgetClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> PlayerWidgetClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UPlayerMinimapIcon> PlayerNicknameMinimapClass;

	UPROPERTY(Transient)
	FBoundingBox BoundingBox;

	UPROPERTY(EditAnywhere)
	UMaterial* ExplorationMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTextureRenderTarget2D* ExplorationMask;

	UFUNCTION()
	UCanvasPanelSlot* CreateWidgetInPanel(const FVector2D& Position, UTexture2D* TileTexture);

	UFUNCTION()
	void AddTile(const FVector2D& Position, UTexture2D* TileTexture);

	UFUNCTION()
	void CalculateSize();

	UFUNCTION()
	void CalculateBoundingBox(const TArray<FLocatedTile>& Layout);

	UFUNCTION()
	void CalculateTileSize();

	UFUNCTION()
	FVector2D LayoutPositionToCanvasPosition(const FVector2D& LayoutPosition) const;

	UFUNCTION()
	FVector2D WorldPositionToCanvasPosition(const FVector& Location) const;

	UFUNCTION()
	void UpdatePlayerLocation(FPlayerInMinimap& PlayerIcon);

	UFUNCTION()
	bool ContainsPlayerIcon(APawn* Player);

	UFUNCTION()
	float GetPlayerWidgetSize() const;

	UFUNCTION()
	void AddPlayerNicknameWidget(FPlayerInMinimap& PlayerIcon, const FString& PlayerNickname) const;

	UFUNCTION()
	FVector2D GetCanvasCenter() const;

	UFUNCTION()
	void CenterCanvasOnPlayer();

	UFUNCTION()
	void TryAddPlayersIconInMap(const TArray<FPlayerInfo>& Descriptions);

public:
	UFUNCTION()
	void RefreshNicknames(const TArray<FPlayerInfo>& Descriptions);

	UFUNCTION()
	void UpdatePlayers();

	UFUNCTION()
	void GenerateMinimap(const TArray<FLocatedTile>& Layout);

	UFUNCTION()
	void ClearMap();

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};