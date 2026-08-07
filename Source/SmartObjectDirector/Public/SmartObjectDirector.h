// Copyright 2026 Silvan Teufel All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * SmartObjectDirector (Runtime) — the runtime extension layer for Epic's Smart Object framework.
 * Provides dynamic runtime registration (USODirectorComponent), a player/AI interaction bridge
 * with server-authoritative claiming and Motion Warping alignment (USOInteractionComponent),
 * and a StateTree task adapter (FSOTask_UseSmartObject) for AI agents.
 * Editor-facing tooling lives in the separate SmartObjectDirectorEditor module.
 */
class FSmartObjectDirectorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
