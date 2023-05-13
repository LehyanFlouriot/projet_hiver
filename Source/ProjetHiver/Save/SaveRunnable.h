#pragma once

#include "CoreMinimal.h"
#include "Core/Public/HAL/Runnable.h"

//Forward Declarations
class FRunnableThread;
class UFileHelperSubsystem;
class UDataBaseSubsystem;
class USaveSubsystem;
struct FFilesToSave;

class FSaveRunnable : public FRunnable
{
	TQueue<FFilesToSave, EQueueMode::Spsc>* Queue;
	FEventRef* EventSave;
	const USaveSubsystem* SaveSubsystem;
	
	bool bStopThread = false;

	// 0 write on disk, 1 store on data base
	int32 SaveType = 0;

	FRunnableThread* Thread = nullptr;

public:
	FSaveRunnable(TQueue<FFilesToSave, EQueueMode::Spsc>* _Queue,
		FEventRef* _Event, const USaveSubsystem* _SaveSubsystem);
	~FSaveRunnable();

	void Launch();
	void SetSaveType(int32 Type) { SaveType = Type; }

	// FRunnable functions
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

private:
	void SaveFiles() const noexcept;
};