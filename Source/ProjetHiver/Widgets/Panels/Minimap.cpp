#include "Minimap.h"

#include "ProjetHiver/EfhorisCore/EfhorisGameState.h"
#include "Components/CanvasPanelSlot.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerState.h"
#include "ProjetHiver/MapGeneration/LevelGenerator.h"
#include "ProjetHiver/Widgets/PlayerMinimapIcon.h"
#include "ProjetHiver/Widgets/TileMinimapWidget.h"

void UMinimap::GenerateMinimap(const TArray<FLocatedTile>& Layout) {
	ClearMap();
	CalculateBoundingBox(Layout);
	CalculateSize();
	CalculateTileSize();
	for (const auto& LocatedTile : Layout) {
		AddTile(LocatedTile.Position, LocatedTile.Texture);
	}
}

UCanvasPanelSlot* UMinimap::CreateWidgetInPanel(const FVector2D& Position, UTexture2D* TileTexture) {
	if (IsValid(TilesCanvasPanel))
		if (UTileMinimapWidget* TileWidget = Cast<UTileMinimapWidget>(CreateWidget(TilesCanvasPanel, TileMinimapWidgetClass)); IsValid(TileWidget))
		{
			TileWidget->SetTileTexture(TileTexture);

			UCanvasPanelSlot* CanvasSlot = TilesCanvasPanel->AddChildToCanvas(TileWidget);
			CanvasSlot->SetPosition(Position);
			CanvasSlot->SetSize(FVector2D(TileSize,TileSize));
			return CanvasSlot;
		}
	return nullptr;
}

void UMinimap::AddTile(const FVector2D& Position, UTexture2D* TileTexture) {
	if (UCanvasPanelSlot* Widget = CreateWidgetInPanel(LayoutPositionToCanvasPosition(Position), TileTexture); IsValid(Widget)) {
		FMinimapTile MinimapTile;
		MinimapTile.Position = Position;
		MinimapTile.Slot = Widget;
		Tiles.Add(MinimapTile);
	}
}

void UMinimap::CalculateSize() {
	SizeX = BoundingBox.MaxX - BoundingBox.MinX + 1;
	SizeY = BoundingBox.MaxY - BoundingBox.MinY + 1;
}

void UMinimap::CalculateBoundingBox(const TArray<FLocatedTile>& Layout) {
	if (Layout.IsEmpty())
	{
		BoundingBox.MinX = 0;
		BoundingBox.MaxX = 0;
		BoundingBox.MinY = 0;
		BoundingBox.MaxY = 0;
		return;
	}

	const auto& FirstTile = Layout[0];
	int MinX = FirstTile.Position.X;
	int MaxX = MinX;
	int MinY = FirstTile.Position.Y;
	int MaxY = MinY;
	for (const auto& Tile : Layout) {
		MinX = FMath::Min(Tile.Position.X, MinX);
		MaxX = FMath::Max(Tile.Position.X, MaxX);
		MinY = FMath::Min(Tile.Position.Y, MinY);
		MaxY = FMath::Max(Tile.Position.Y, MaxY);
	}
	BoundingBox.MinX = MinX;
	BoundingBox.MaxX = MaxX;
	BoundingBox.MinY = MinY;
	BoundingBox.MaxY = MaxY;
}

void UMinimap::CalculateTileSize() {
	const int MaxSize = FMath::Max(SizeX, SizeY);
	TileSize = CanvasSize / MaxSize ;
}

FVector2D UMinimap::LayoutPositionToCanvasPosition(const FVector2D& LayoutPosition) const {
	const float OffsetX =  SizeX < SizeY ? (SizeY - SizeX) / 2.0f : 0.0f;
	const float OffsetY = SizeY < SizeX ? (SizeX - SizeY) / 2.0f : 0.0f;
	return TileSize * FVector2D(LayoutPosition.X - BoundingBox.MinX + OffsetX, LayoutPosition.Y - BoundingBox.MinY + OffsetY);
}

FVector2D UMinimap::WorldPositionToCanvasPosition(const FVector& Location) const {
	constexpr  float WorldTileSize = 6000.0f;
	constexpr float InTileOffset = WorldTileSize / 2;
	const FVector2D LayoutPosition = FVector2D((Location.X + InTileOffset) / WorldTileSize , (Location.Y + InTileOffset) / WorldTileSize);
	return LayoutPositionToCanvasPosition(LayoutPosition);
}

void UMinimap::UpdatePlayerLocation(FPlayerInMinimap& PlayerIcon) {
	if (!IsValid(PlayerIcon.Player)) return;

	if (!IsValid(PlayerIcon.Widget)) return;
	const FVector2D CanvasLocation = WorldPositionToCanvasPosition(PlayerIcon.Player->GetActorLocation());
	PlayerIcon.Widget->SetPosition(CanvasLocation);

	if (!IsValid(PlayerIcon.NicknameWidget)) return;
	const FVector2D PlayerNicknameOffset = FVector2D(0, - GetPlayerWidgetSize());
	PlayerIcon.NicknameWidget->SetPosition(CanvasLocation + PlayerNicknameOffset);

	if (!IsValid(ExplorationMask) || !IsValid(ExplorationMaterial)) return;
	UMaterialInstanceDynamic* ExplorationMaterialInstance = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), ExplorationMaterial);
	ExplorationMaterialInstance->SetVectorParameterValue("Location", FVector(CanvasLocation.X / CanvasSize, CanvasLocation.Y / CanvasSize,0));
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), ExplorationMask, ExplorationMaterialInstance);
}

bool UMinimap::ContainsPlayerIcon(APawn* Player) {
	for (const auto& PlayerIcon : PlayersInMap)
	{
		if (PlayerIcon.Player == Player)
			return true;
	}
	return false;
}

float UMinimap::GetPlayerWidgetSize() const {
	return PlayerWidgetSize;
}

void UMinimap::AddPlayerNicknameWidget(FPlayerInMinimap& PlayerIcon, const FString& PlayerNickname) const {
	if (UPlayerMinimapIcon* PlayerNicknameWidget = Cast<UPlayerMinimapIcon>(CreateWidget(TilesCanvasPanel, PlayerNicknameMinimapClass)); IsValid(PlayerNicknameWidget)) {
		if (!PlayerNickname.IsEmpty())
		{
			PlayerNicknameWidget->SetNickname(PlayerNickname);
		}

		UCanvasPanelSlot* NicknameCanvasSlot = IconsCanvasPanel->AddChildToCanvas(PlayerNicknameWidget);
		PlayerIcon.NicknameWidget = NicknameCanvasSlot;
		NicknameCanvasSlot->SetSize(FVector2D(10 * PlayerNickname.Len(), 15));
		NicknameCanvasSlot->SetAlignment(FVector2D(0.5, 0.5));
	}
}

FVector2D UMinimap::GetCanvasCenter() const {
	const float OffsetX = SizeX < SizeY ? (SizeY - SizeX) / 2.0f : 0.0f;
	const float OffsetY = SizeY < SizeX ? (SizeX - SizeY) / 2.0f : 0.0f;
	return TileSize / 2 * FVector2D(SizeX + OffsetX,SizeY + OffsetY);
}

void UMinimap::CenterCanvasOnPlayer() {
	const FVector PlayerWorldLocation = GetOwningPlayerPawn()->GetActorLocation();
	const FVector2D PlayerCanvasLocation = WorldPositionToCanvasPosition(PlayerWorldLocation);
	RetainerBox->SetRenderTranslation(-PlayerCanvasLocation + GetCanvasCenter());
}

void UMinimap::TryAddPlayersIconInMap(const TArray<FPlayerInfo>& Descriptions) {
	for (auto& PlayerInfo : Descriptions) {
		if (APawn* Pawn = PlayerInfo.Character; IsValid(Pawn) && !ContainsPlayerIcon(Pawn)) {
			if (const APlayerCharacter* Player = Cast<APlayerCharacter>(Pawn); IsValid(Player) && Player->IsAliveTag()) {
				FPlayerInMinimap PlayerIcon;
				PlayerIcon.Player = Pawn;

				if (UUserWidget* Widget = Cast<UUserWidget>(CreateWidget(TilesCanvasPanel, PlayerWidgetClass)); IsValid(Widget)) {

					AddPlayerNicknameWidget(PlayerIcon,PlayerInfo.NickName);

					UCanvasPanelSlot* CanvasSlot = IconsCanvasPanel->AddChildToCanvas(Widget);
					PlayerIcon.Widget = CanvasSlot;
					CanvasSlot->SetSize(GetPlayerWidgetSize() * FVector2D(1, 1));
					CanvasSlot->SetAlignment(FVector2D(0.5, 0.5));

					PlayerIcon.PlayerIconWidget = Widget;

					PlayersInMap.Add(PlayerIcon);
				}
			}
		}
	}
}

void UMinimap::RefreshNicknames(const TArray<FPlayerInfo>& Descriptions) {
	IconsCanvasPanel->ClearChildren();
	PlayersInMap.Empty();
	TryAddPlayersIconInMap(Descriptions);
}

void UMinimap::UpdatePlayers() {
	for (FPlayerInMinimap& Icon : PlayersInMap)
	{
		if (!IsValid(Icon.Player)) continue;
		if (const APlayerCharacter* Player = Cast<APlayerCharacter>(Icon.Player); IsValid(Player) && !Player->IsAliveTag()) continue;
		UpdatePlayerLocation(Icon);
	}
}

void UMinimap::ClearMap() {
	TilesCanvasPanel->ClearChildren();
	IconsCanvasPanel->ClearChildren();
	Tiles.Empty();
	PlayersInMap.Empty();
	UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), ExplorationMask, FLinearColor(0, 0, 0, 0));
}

void UMinimap::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!IsValid(TilesCanvasPanel)) return;

	TArray<FPlayerInMinimap> ToRemove;
	//Tick player icons
	for (auto PlayerIcon : PlayersInMap)
	{
		if (!IsValid(PlayerIcon.Player))
		{
			ToRemove.Add(PlayerIcon);
			continue;
		}

		/*if (!IsValid(PlayerIcon.NicknameWidget)) {
			AddPlayerNicknameWidget(PlayerIcon);
		}*/
	}

	//Remove dead players
	for (auto& PlayerIcon : ToRemove)
	{
		PlayersInMap.Remove(PlayerIcon);
	}

	CenterCanvasOnPlayer();
}
