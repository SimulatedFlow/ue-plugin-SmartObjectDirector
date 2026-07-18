// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "SODirectorComponentVisualizer.h"

#include "SODirectorComponent.h"

#include "SmartObjectDefinition.h"
#include "SmartObjectSubsystem.h"
#include "SmartObjectRuntime.h"

#include "SceneManagement.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

void FSODirectorComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const USODirectorComponent* Director = Cast<USODirectorComponent>(Component);
	if (!Director || !PDI)
	{
		return;
	}

	const USmartObjectDefinition* Definition = Director->GetDefinition();
	const AActor* Owner = Director->GetOwner();
	if (!Definition || !Owner)
	{
		return;
	}

	const FTransform OwnerTransform = Owner->GetActorTransform();

	// If we are in a running world with a live subsystem, query the actual runtime slot states.
	const UWorld* World = Director->GetWorld();
	USmartObjectSubsystem* Subsystem = (World && World->IsGameWorld()) ? USmartObjectSubsystem::GetCurrent(World) : nullptr;

	TArray<FSmartObjectSlotHandle> RuntimeSlots;
	if (Subsystem && Director->IsRegistered())
	{
		Subsystem->GetAllSlots(Director->GetRegisteredHandle(), RuntimeSlots);
	}

	const TConstArrayView<FSmartObjectSlotDefinition> Slots = Definition->GetSlots();
	for (int32 SlotIndex = 0; SlotIndex < Slots.Num(); ++SlotIndex)
	{
		const FTransform SlotWorld = Definition->GetSlotWorldTransform(SlotIndex, OwnerTransform);
		const FVector Location = SlotWorld.GetLocation();

		// Default: color from the definition's initial enabled state (Green = enabled, Orange = disabled).
		FColor Color = Slots[SlotIndex].bEnabled ? FColor::Green : FColor::Orange;

		// Override with live runtime state when available.
		if (Subsystem && RuntimeSlots.IsValidIndex(SlotIndex))
		{
			const ESmartObjectSlotState State = Subsystem->GetSlotState(RuntimeSlots[SlotIndex]);
			if (State == ESmartObjectSlotState::Claimed || State == ESmartObjectSlotState::Occupied)
			{
				Color = FColor::Red;
			}
			else if (State == ESmartObjectSlotState::Free)
			{
				Color = Slots[SlotIndex].bEnabled ? FColor::Green : FColor::Orange;
			}
		}

		// Draw the entry point marker and its facing direction.
		PDI->DrawPoint(Location, Color, 14.0f, SDPG_Foreground);
		DrawWireSphere(PDI, Location, Color, 20.0f, 12, SDPG_World, 1.5f);

		const FVector Forward = SlotWorld.GetRotation().GetForwardVector();
		PDI->DrawLine(Location, Location + Forward * 45.0f, Color, SDPG_World, 2.0f);
	}
}
