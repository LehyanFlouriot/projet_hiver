#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "InteractionComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJETHIVER_API UInteractionComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractionComponent();

	UFUNCTION(Server, Reliable)
	void Server_StartInteract();

	UFUNCTION(Server, Reliable)
	void Server_StopInteract();

	UFUNCTION(Server, Reliable)
	void Server_ForceStopInteract();

	void Authority_StartInteract();
	void Authority_StopInteract() const;
	void Authority_ForceStopInteract() const;

protected:
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION()
	bool IsInteractible(const APlayerCharacter* InteractionSource,AActor* InteractionTarget) const;

	UFUNCTION()
	void OnOwnerEndOverlap(AActor* OwningActor, AActor* OtherActor);

	void OnSetupPlayerInput(UInputComponent* InputComponent);

	AActor* FindInteractiveActorWithRaycast(const APlayerCharacter* Source) const noexcept;
	void RemoveInteractiveActorInRange(AActor* Actor);

	UFUNCTION(BlueprintCallable)
	AActor* GetInteractiveTarget() const noexcept { return InteractiveTarget;}

	UPROPERTY(EditAnywhere)
	float InteractionRange = 150.0f;

	UPROPERTY(Transient, Replicated)
	AActor* InteractiveTarget;
};
