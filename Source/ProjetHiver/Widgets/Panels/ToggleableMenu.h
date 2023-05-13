#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ToggleableMenu.generated.h"

UCLASS()
class PROJETHIVER_API UToggleableMenu : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite)
	UWidgetAnimation* OpenAnim;

	UPROPERTY(Transient)
	FWidgetAnimationDynamicEvent OpenAnimEndDelegate;

	UPROPERTY(BlueprintReadWrite)
	UWidgetAnimation* CloseAnim;

	UPROPERTY(Transient)
	FWidgetAnimationDynamicEvent CloseAnimEndDelegate;

	UPROPERTY(Transient)
	bool bIsInCooldown;

	UPROPERTY(Transient,BlueprintReadOnly)
	bool bIsOpened;

	UPROPERTY(EditAnywhere)
	bool bAffectPlayerMode = false;

	UPROPERTY(EditAnywhere)
	bool bIsInteraction = false;

	UPROPERTY(EditAnywhere)
	bool bIsMerchant = false;

	UPROPERTY(EditAnywhere)
	bool bIsSmith = false;

	UFUNCTION()
	void OpenAnimEnd();

	UFUNCTION()
	void CloseAnimEnd();

	UFUNCTION(BlueprintCallable)
	void InitAnimations(UWidgetAnimation* OpenAnimation, UWidgetAnimation* CloseAnimation);

	UFUNCTION()
	void ForceCloseMenuInstant();

	UFUNCTION()
	void ForceCloseMenuWithAnimation();

public:
	UFUNCTION(BlueprintCallable)
	void OpenMenu();

	UFUNCTION(BlueprintCallable)
	void CloseMenu(const bool bIgnoreCooldown = false, const bool bIgnoreCooldownANDPlayAnimation = false);

	UFUNCTION(BlueprintCallable)
	void ForceCloseMenu(const bool bPlayAnimation = false);
};
