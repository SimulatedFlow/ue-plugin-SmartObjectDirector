// Copyright 2026 Simulated Flow All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"

/**
 * Draws the dynamic entry points of a USODirectorComponent in the editor viewport and colors
 * them by state: Green = Free, Red = Claimed/Occupied, Orange = Disabled. During PIE the live
 * runtime slot state is queried from the Smart Object subsystem; in the editor the slot's initial
 * enabled state from the definition asset is used.
 */
class FSODirectorComponentVisualizer : public FComponentVisualizer
{
public:
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
};
