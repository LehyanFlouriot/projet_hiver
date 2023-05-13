// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjetHiver/GAS/EfhorisBlueprintAsyncActionBase.h"

UEfhorisBlueprintAsyncActionBase* UEfhorisBlueprintAsyncActionBase::ListenForAttributeChange(
	UAbilitySystemComponent* AbilitySystemComponent, FGameplayAttribute Attribute)
{
	UEfhorisBlueprintAsyncActionBase* WaitForAttributeChangedTask = NewObject<UEfhorisBlueprintAsyncActionBase>();
	WaitForAttributeChangedTask->ASC = AbilitySystemComponent;
	WaitForAttributeChangedTask->AttributeToListenFor = Attribute;

	if (!IsValid(AbilitySystemComponent) || !Attribute.IsValid())
	{
		WaitForAttributeChangedTask->RemoveFromRoot();
		return nullptr;
	}

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(WaitForAttributeChangedTask, &UEfhorisBlueprintAsyncActionBase::AttributeChanged);

	return WaitForAttributeChangedTask;
}

UEfhorisBlueprintAsyncActionBase* UEfhorisBlueprintAsyncActionBase::ListenForAttributesChange(UAbilitySystemComponent* AbilitySystemComponent, TArray<FGameplayAttribute> Attributes)
{
	UEfhorisBlueprintAsyncActionBase* WaitForAttributeChangedTask = NewObject<UEfhorisBlueprintAsyncActionBase>();
	WaitForAttributeChangedTask->ASC = AbilitySystemComponent;
	WaitForAttributeChangedTask->AttributesToListenFor = Attributes;

	if (!IsValid(AbilitySystemComponent) || Attributes.Num() < 1)
	{
		WaitForAttributeChangedTask->RemoveFromRoot();
		return nullptr;
	}

	for (FGameplayAttribute Attribute : Attributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(WaitForAttributeChangedTask, &UEfhorisBlueprintAsyncActionBase::AttributeChanged);
	}

	return WaitForAttributeChangedTask;
}

void UEfhorisBlueprintAsyncActionBase::EndTask()
{
	if (IsValid(ASC))
	{
		ASC->GetGameplayAttributeValueChangeDelegate(AttributeToListenFor).RemoveAll(this);

		for (FGameplayAttribute Attribute : AttributesToListenFor)
		{
			ASC->GetGameplayAttributeValueChangeDelegate(Attribute).RemoveAll(this);
		}
	}

	SetReadyToDestroy();
	MarkAsGarbage();
}

void UEfhorisBlueprintAsyncActionBase::AttributeChanged(const FOnAttributeChangeData& Data)
{
	OnAttributeChanged.Broadcast(Data.Attribute, Data.NewValue, Data.OldValue);
}
