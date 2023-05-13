#include "ToggleableMenu.h"

#include "Animation/WidgetAnimation.h"
#include "ProjetHiver/Characters/PlayerCharacter.h"
#include "ProjetHiver/EfhorisCore/EfhorisPlayerController.h"

void UToggleableMenu::OpenAnimEnd() {
	bIsInCooldown = false;
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOwningPlayer()); IsValid(PlayerController))
	{
		PlayerController->Server_IncrementMenuCooldown(false);
	}
}

void UToggleableMenu::CloseAnimEnd() {
	bIsInCooldown = false;
	SetVisibility(ESlateVisibility::Hidden);
	if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOwningPlayer()); IsValid(PlayerController))
	{
		PlayerController->Server_IncrementMenuCooldown(false);
	}
}

void UToggleableMenu::InitAnimations(UWidgetAnimation* OpenAnimation, UWidgetAnimation* CloseAnimation) {
	OpenAnim = OpenAnimation;
	OpenAnimEndDelegate.BindDynamic(this, &UToggleableMenu::OpenAnimEnd);
	if (OpenAnim) {
		BindToAnimationFinished(OpenAnim, OpenAnimEndDelegate);
	}

	CloseAnim = CloseAnimation;
	CloseAnimEndDelegate.BindDynamic(this, &UToggleableMenu::CloseAnimEnd);
	if (CloseAnim) {
		BindToAnimationFinished(CloseAnim, CloseAnimEndDelegate);
	}
}

void UToggleableMenu::OpenMenu() {
	if (!bIsInCooldown && !bIsOpened) {
		bIsOpened = true;
		SetVisibility(ESlateVisibility::Visible);
		//Use open animation if present
		if (OpenAnim) {
			bIsInCooldown = true;
			if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOwningPlayer()); IsValid(PlayerController))
			{
				PlayerController->Server_IncrementMenuCooldown(true);
			}
			PlayAnimation(OpenAnim);
		}
		//Change the player play mode to UI if needed
		if (bAffectPlayerMode) {
			if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOwningPlayer()); IsValid(PlayerController))
			{
				PlayerController->AddOpenMenu();
			}
		}
		//Set is interacting according to the current menu
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOwningPlayerPawn()); IsValid(PlayerCharacter))
		{
			if (bIsInteraction) {
				PlayerCharacter->Server_AddInteraction();
			}
			if (bIsMerchant) {
				PlayerCharacter->Server_SetIsTrading(true);
			}
			if(bIsSmith) {
				PlayerCharacter->Server_SetIsSmithing(true);
			}
			
		}
	}
}

void UToggleableMenu::CloseMenu(const bool bIgnoreCooldown, const bool bIgnoreCooldownANDPlayAnimation) {
	if ((!bIsInCooldown || bIgnoreCooldown ||bIgnoreCooldownANDPlayAnimation) && bIsOpened) {
		bIsOpened = false;
		if (!CloseAnim || bIgnoreCooldown) {
			bIsInCooldown = false;
			SetVisibility(ESlateVisibility::Hidden);
		}
		else if (CloseAnim || (bIgnoreCooldownANDPlayAnimation && CloseAnim)) {
			//Use close animation if present
			bIsInCooldown = true;
			if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOwningPlayer()); IsValid(PlayerController))
			{
				PlayerController->Server_IncrementMenuCooldown(true);
			}
			PlayAnimation(CloseAnim);
		}
		//Change the player play mode to Game if needed
		if (bAffectPlayerMode) {
			if (AEfhorisPlayerController* PlayerController = Cast<AEfhorisPlayerController>(GetOwningPlayer()); IsValid(PlayerController))
			{
				PlayerController->RemoveOpenMenu();
			}
		}
		//Set is interacting according to the current menu
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOwningPlayerPawn()); IsValid(PlayerCharacter))
		{
			if (bIsInteraction) {
				PlayerCharacter->Server_RemoveInteraction();
			}
			if (bIsMerchant) {
				PlayerCharacter->Server_SetIsTrading(false);
			}
			if (bIsSmith) {
				PlayerCharacter->Server_SetIsSmithing(false);
			}

		}
	}
}

void UToggleableMenu::ForceCloseMenu(const bool bPlayAnimation) {
	if (bPlayAnimation)
		ForceCloseMenuWithAnimation();
	else
		ForceCloseMenuInstant();
}

void UToggleableMenu::ForceCloseMenuInstant() {
	CloseMenu(true);
}

void UToggleableMenu::ForceCloseMenuWithAnimation() {
	CloseMenu(false, true);
}
