// Copyright 2026 Simulated Flow All Rights Reserved.

#include "SmartObjectDirectorEditor.h"
#include "SODirectorComponentVisualizer.h"
#include "SODirectorComponent.h"
#include "SmartObjectDirectorLog.h"

#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"

#define LOCTEXT_NAMESPACE "FSmartObjectDirectorEditorModule"

void FSmartObjectDirectorEditorModule::StartupModule()
{
	if (GUnrealEd)
	{
		TSharedPtr<FComponentVisualizer> Visualizer = MakeShareable(new FSODirectorComponentVisualizer());
		GUnrealEd->RegisterComponentVisualizer(USODirectorComponent::StaticClass()->GetFName(), Visualizer);
		Visualizer->OnRegister();
	}

	UE_LOG(LogSmartObjectDirector, Log, TEXT("SmartObjectDirectorEditor started."));
}

void FSmartObjectDirectorEditorModule::ShutdownModule()
{
	if (GUnrealEd)
	{
		GUnrealEd->UnregisterComponentVisualizer(USODirectorComponent::StaticClass()->GetFName());
	}

	UE_LOG(LogSmartObjectDirector, Log, TEXT("SmartObjectDirectorEditor shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSmartObjectDirectorEditorModule, SmartObjectDirectorEditor)
