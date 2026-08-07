// Copyright 2026 Silvan Teufel All Rights Reserved.

#include "SODirectorComponent.h"
#include "SmartObjectDirectorLog.h"

#include "SmartObjectSubsystem.h"
#include "SmartObjectDefinition.h"
#include "SmartObjectTypes.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

USODirectorComponent::USODirectorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;
}

void USODirectorComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoRegisterOnBeginPlay)
	{
		RegisterWithSubsystem();
	}
}

void USODirectorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterFromSubsystem();
	Super::EndPlay(EndPlayReason);
}

USmartObjectSubsystem* USODirectorComponent::GetSubsystem() const
{
	const UWorld* World = GetWorld();
	return World ? USmartObjectSubsystem::GetCurrent(World) : nullptr;
}

const USmartObjectDefinition* USODirectorComponent::GetDefinition() const
{
	if (SmartObjectDefinitionAsset.IsNull())
	{
		return nullptr;
	}

	// Synchronous load is acceptable here: definitions are small data assets and this is
	// only hit on registration / editor visualization, not on a hot path.
	return SmartObjectDefinitionAsset.LoadSynchronous();
}

bool USODirectorComponent::RegisterWithSubsystem()
{
	if (RegisteredHandle.IsValid())
	{
		return true;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	USmartObjectSubsystem* Subsystem = GetSubsystem();
	if (!Subsystem)
	{
		UE_LOG(LogSmartObjectDirector, Warning, TEXT("[%s] RegisterWithSubsystem: no SmartObjectSubsystem available."), *GetNameSafe(Owner));
		return false;
	}

	const USmartObjectDefinition* Definition = GetDefinition();
	if (!Definition)
	{
		UE_LOG(LogSmartObjectDirector, Warning, TEXT("[%s] RegisterWithSubsystem: no SmartObjectDefinition assigned."), *GetNameSafe(Owner));
		return false;
	}

	// Create a dynamic runtime instance whose lifetime we control explicitly.
	RegisteredHandle = Subsystem->CreateSmartObject(
		*Definition,
		Owner->GetActorTransform(),
		FConstStructView::Make(FSmartObjectActorOwnerData(Owner)));

	if (RegisteredHandle.IsValid())
	{
		UE_LOG(LogSmartObjectDirector, Verbose, TEXT("[%s] Registered dynamic Smart Object %s."), *GetNameSafe(Owner), *LexToString(RegisteredHandle));
		return true;
	}

	UE_LOG(LogSmartObjectDirector, Warning, TEXT("[%s] RegisterWithSubsystem: CreateSmartObject failed."), *GetNameSafe(Owner));
	return false;
}

void USODirectorComponent::UnregisterFromSubsystem()
{
	if (!RegisteredHandle.IsValid())
	{
		return;
	}

	if (USmartObjectSubsystem* Subsystem = GetSubsystem())
	{
		Subsystem->DestroySmartObject(RegisteredHandle);
	}

	RegisteredHandle.Invalidate();
}

void USODirectorComponent::GatherSlotsForTag(FGameplayTag SlotTag, TArray<FSmartObjectSlotHandle>& OutSlots) const
{
	OutSlots.Reset();

	USmartObjectSubsystem* Subsystem = GetSubsystem();
	const USmartObjectDefinition* Definition = GetDefinition();
	if (!Subsystem || !Definition || !RegisteredHandle.IsValid())
	{
		return;
	}

	TArray<FSmartObjectSlotHandle> AllSlots;
	Subsystem->GetAllSlots(RegisteredHandle, AllSlots);

	const TConstArrayView<FSmartObjectSlotDefinition> SlotDefinitions = Definition->GetSlots();

	// Runtime slot handles are ordered by slot index, matching the definition's slot array.
	for (const FSmartObjectSlotHandle& SlotHandle : AllSlots)
	{
		const int32 SlotIndex = SlotHandle.GetSlotIndex();
		if (!SlotDefinitions.IsValidIndex(SlotIndex))
		{
			continue;
		}

		// An unset filter tag matches every slot; otherwise require the slot's activity tags to contain it.
		if (!SlotTag.IsValid() || SlotDefinitions[SlotIndex].ActivityTags.HasTag(SlotTag))
		{
			OutSlots.Add(SlotHandle);
		}
	}
}

bool USODirectorComponent::SetSlotEnabled(FGameplayTag SlotTag, bool bEnabled)
{
	USmartObjectSubsystem* Subsystem = GetSubsystem();
	if (!Subsystem)
	{
		return false;
	}

	TArray<FSmartObjectSlotHandle> Slots;
	GatherSlotsForTag(SlotTag, Slots);

	bool bAnyChanged = false;
	for (const FSmartObjectSlotHandle& SlotHandle : Slots)
	{
		Subsystem->SetSlotEnabled(SlotHandle, bEnabled);
		bAnyChanged = true;
	}

	return bAnyChanged;
}

bool USODirectorComponent::IsSlotClaimed(FGameplayTag SlotTag) const
{
	USmartObjectSubsystem* Subsystem = GetSubsystem();
	if (!Subsystem)
	{
		return false;
	}

	TArray<FSmartObjectSlotHandle> Slots;
	GatherSlotsForTag(SlotTag, Slots);

	for (const FSmartObjectSlotHandle& SlotHandle : Slots)
	{
		const ESmartObjectSlotState State = Subsystem->GetSlotState(SlotHandle);
		if (State == ESmartObjectSlotState::Claimed || State == ESmartObjectSlotState::Occupied)
		{
			return true;
		}
	}

	return false;
}
