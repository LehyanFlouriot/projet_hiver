// Fill out your copyright notice in the Description page of Project Settings.


#include "SoundComponent.h"

#include "AkAudioEvent.h"
#include "AkGameplayTypes.h"
#include "../../../Plugins/Wwise/Source/AkAudio/Classes/AkGameplayStatics.h"

// Sets default values for this component's properties
USoundComponent::USoundComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void USoundComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void USoundComponent::UninitializeComponent()
{
	Super::UninitializeComponent();
}

void USoundComponent::PlaySound(UAkAudioEvent* Event, AActor* Actor) const
{
	if(Actor == nullptr)
		FAkAudioDevice::Get()->PostEvent(Event, GetOwner());
		//UAkGameplayStatics::PostEvent(Event, GetOwner(), 0, {});
	else
		FAkAudioDevice::Get()->PostEvent(Event, Actor);
		//UAkGameplayStatics::PostEvent(Event, Actor, 0, {});
}

void USoundComponent::PlaySoundAtLocation(UAkAudioEvent* Event, const FVector Location, const FRotator Rotation) const
{
	FAkAudioDevice::Get()->PostEventAtLocation(Event, Location, Rotation, GetWorld(), EAkAudioContext::GameplayAudio);
	//UAkGameplayStatics::PostEventAtLocation(Event, Location, Rotation, Event->GetName(), GetWorld());
}

void USoundComponent::PlaySoundAttached(UAkAudioEvent* Event, AActor* Actor) const
{
	if (Actor == nullptr)
		FAkAudioDevice::Get()->PostEvent(Event, GetOwner());
		//UAkGameplayStatics::PostEventAttached(Event, GetOwner(), {""}, true);
	else
		FAkAudioDevice::Get()->PostEvent(Event, Actor);
		//UAkGameplayStatics::PostEventAttached(Event, Actor, {""}, true);

}

void USoundComponent::PlayStoppableSound(UAkAudioEvent* Event, AActor* Actor)
{
	if(Actor == nullptr)
		SoundMap.Add(Event, UAkGameplayStatics::PostEvent(Event, GetOwner(), 0, {}));
	else
		SoundMap.Add(Event, UAkGameplayStatics::PostEvent(Event, Actor, 0, {}));
}

void USoundComponent::PlayStoppableSoundAtLocation(UAkAudioEvent* Event, const FVector Location, const FRotator Rotation)
{
	SoundMap.Add(Event, UAkGameplayStatics::PostEventAtLocation(Event, Location, Rotation, Event->GetName(), GetWorld()));
}

void USoundComponent::PlayStoppableSoundAttached(UAkAudioEvent* Event, AActor* Actor)
{
	if(Actor == nullptr)
		SoundMap.Add(Event, UAkGameplayStatics::PostEventAttached(Event, GetOwner(), {""}, true));
	else
		SoundMap.Add(Event, UAkGameplayStatics::PostEventAttached(Event, Actor, {""}, true));
}

void USoundComponent::ActOnSound(UAkAudioEvent* Event, const AkActionOnEventType Action)
{
	if (const int32* Value = SoundMap.Find(Event))
		UAkGameplayStatics::ExecuteActionOnPlayingID(Action, *Value);
}
