#include "ProjetHiver/EfhorisCore/EfhorisHUD.h"

#include "EfhorisPlayerController.h"
#include "ProjetHiver/Widgets/GameView.h"

UGameView* AEfhorisHUD::GetGameView() const
{
	return GameView;
}

void AEfhorisHUD::HideGameView()
{
	GameView->SetVisibility(ESlateVisibility::Hidden);
}

void AEfhorisHUD::ShowGameView()
{
	GameView->SetVisibility(ESlateVisibility::Visible);
}

void AEfhorisHUD::Win()
{
	ShowGameView();
	GetGameView()->Win();
}

//
//void AEfhorisHUD::ShowHUD()
//{
//	Super::ShowHUD();
//	GameView->SetIsEnabled(!GameView->GetIsEnabled());
//}
