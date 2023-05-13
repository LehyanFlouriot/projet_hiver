// Fill out your copyright notice in the Description page of Project Settings.

#include "Signal.h"
#include "ProjetHiver/Characters/BaseCharacter.h"

void USignal::Init(ESignalType SignalType_, float Radius_, ABaseCharacter* Emitter_)
{
	SignalType = SignalType_;
	Radius = Radius_;
	Emitter = Emitter_;
}
