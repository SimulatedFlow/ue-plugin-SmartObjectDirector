// Copyright 2026 Simulated Flow All Rights Reserved.

#include "SmartObjectDirector.h"
#include "SmartObjectDirectorLog.h"

#define LOCTEXT_NAMESPACE "FSmartObjectDirectorModule"

void FSmartObjectDirectorModule::StartupModule()
{
	UE_LOG(LogSmartObjectDirector, Log, TEXT("SmartObjectDirector runtime module started."));
}

void FSmartObjectDirectorModule::ShutdownModule()
{
	UE_LOG(LogSmartObjectDirector, Log, TEXT("SmartObjectDirector runtime module shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSmartObjectDirectorModule, SmartObjectDirector)
