#include "ProjetHiver/Save/SaveRunnable.h"

#include "ProjetHiver/Save/FileHelperSubsystem.h"
#include "ProjetHiver/Save/FilesToSave.h"
#include "ProjetHiver/Save/SaveSubsystem.h"
#include "ProjetHiver/BDD/DataBaseSubsystem.h"

FSaveRunnable::FSaveRunnable(TQueue<FFilesToSave, EQueueMode::Spsc>* _Queue,
	FEventRef* _Event, const USaveSubsystem* _SaveSubsystem)
	: Queue{ _Queue }, EventSave{ _Event }, SaveSubsystem{ _SaveSubsystem } {
	
}

FSaveRunnable::~FSaveRunnable()
{
	if (Thread != nullptr)
	{
		Thread->Kill(true);
		delete Thread;
	}
}

void FSaveRunnable::Launch() {
	Thread = FRunnableThread::Create(this, TEXT("Lancement Thread Sauvegarde"));
}

uint32 FSaveRunnable::Run()
{
	UE_LOG(LogTemp, Warning, TEXT("Save Thread Begin"));

	bStopThread = false;
	(*EventSave)->Wait();
	while(!bStopThread)
	{
		SaveFiles();
		(*EventSave)->Wait();
	}

	return 0;
}

void FSaveRunnable::Stop()
{
	bStopThread = true;
}

void FSaveRunnable::Exit()
{
	UE_LOG(LogTemp, Warning, TEXT("Save Thread End"));
	FRunnable::Exit();
}

void FSaveRunnable::SaveFiles() const noexcept
{
	FFilesToSave FilesToSave;

	while (Queue->Dequeue(FilesToSave)) {
		if (SaveType == 0)
			SaveSubsystem->WriteStatesSave(FilesToSave);
		else if (SaveType == 1)
			SaveSubsystem->StoreStatesSave(FilesToSave);
	}
}
