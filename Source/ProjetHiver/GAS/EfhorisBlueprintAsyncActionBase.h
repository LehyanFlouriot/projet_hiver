// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AbilitySystemComponent.h"
#include "EfhorisBlueprintAsyncActionBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAttributeChanged, FGameplayAttribute, Attribute, float, NewValue, float, OldValue);

/**
 * 
 */
UCLASS(BlueprintType, meta=(ExposedAsyncProxy = AsyncTask))
class PROJETHIVER_API UEfhorisBlueprintAsyncActionBase : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
		FOnAttributeChanged OnAttributeChanged;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
		static UEfhorisBlueprintAsyncActionBase* ListenForAttributeChange(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAttribute Attribute);

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
		static UEfhorisBlueprintAsyncActionBase* ListenForAttributesChange(UAbilitySystemComponent* AbilitySystemComponent, TArray<FGameplayAttribute> Attributes);

	UFUNCTION(BlueprintCallable)
		void EndTask();



protected:
	UPROPERTY()
		UAbilitySystemComponent* ASC;

	FGameplayAttribute AttributeToListenFor;
	TArray<FGameplayAttribute> AttributesToListenFor;

	void AttributeChanged(const FOnAttributeChangeData& Data);
};
