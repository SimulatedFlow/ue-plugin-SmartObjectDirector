// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * SmartObjectDirectorEditor — editor-only tooling for the Smart Object Director plugin.
 * Registers a component visualizer that draws the dynamic entry points and live claim
 * state (Green = Free, Red = Claimed/Occupied, Orange = Disabled) of USODirectorComponent
 * instances directly in the editor viewport for fast debugging of runtime smart objects.
 */
class FSmartObjectDirectorEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
