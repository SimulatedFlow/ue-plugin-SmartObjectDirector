// Copyright 2026 Simulated Flow All Rights Reserved.

#include "SOInteractionComponent.h"
#include "SODirectorComponent.h"
#include "SmartObjectDirectorLog.h"

#include "SmartObjectSubsystem.h"
#include "SmartObjectComponent.h"
#include "SmartObjectRequestTypes.h"
#include "SmartObjectDefinition.h"

#include "MotionWarpingComponent.h"

#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

USOInteractionComponent::USOInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void USOInteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USOInteractionComponent, ActiveSmartObject);
}

void USOInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Always release a held reservation so we never leak a claim on the subsystem.
	if (GetOwnerRole() == ROLE_Authority)
	{
		ReleaseClaim();
	}
	Super::EndPlay(EndPlayReason);
}

USmartObjectSubsystem* USOInteractionComponent::GetSubsystem() const
{
	const UWorld* World = GetWorld();
	return World ? USmartObjectSubsystem::GetCurrent(World) : nullptr;
}

AActor* USOInteractionComponent::FindBestInteractable(float QueryRadius, FGameplayTagContainer FilterTags)
{
	AActor* Owner = GetOwner();
	USmartObjectSubsystem* Subsystem = GetSubsystem();
	if (!Owner || !Subsystem)
	{
		return nullptr;
	}

	const FVector Origin = Owner->GetActorLocation();
	const float Radius = FMath::Max(QueryRadius, 1.0f);

	FSmartObjectRequestFilter Filter;
	Filter.UserTags = FilterTags;

	FSmartObjectRequest Request(FBox::BuildAABB(Origin, FVector(Radius)), Filter);

	const FSmartObjectRequestResult Result = Subsystem->FindSmartObject(Request, Owner);
	if (!Result.IsValid())
	{
		return nullptr;
	}

	if (const USmartObjectComponent* Component = Subsystem->GetSmartObjectComponentByRequestResult(Result))
	{
		return Component->GetOwner();
	}

	return nullptr;
}

bool USOInteractionComponent::ServerClaimAndInteract_Validate(AActor* TargetObject, FGameplayTag SlotTag)
{
	// Basic anti-cheat guardrail: reject obviously invalid targets before running server logic.
	return TargetObject != nullptr;
}

void USOInteractionComponent::ServerClaimAndInteract_Implementation(AActor* TargetObject, FGameplayTag SlotTag)
{
	// Authoritative path only. All reservation logic runs on the server to prevent two clients
	// from claiming the same slot in the same frame; the subsystem processes claims sequentially.
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	USmartObjectSubsystem* Subsystem = GetSubsystem();
	AActor* Owner = GetOwner();
	if (!Subsystem || !Owner || !TargetObject)
	{
		return;
	}

	// Release any prior reservation before starting a new one.
	ReleaseClaim();

	const USODirectorComponent* Director = TargetObject->FindComponentByClass<USODirectorComponent>();
	if (!Director || !Director->IsRegistered())
	{
		UE_LOG(LogSmartObjectDirector, Warning, TEXT("ServerClaimAndInteract: target %s has no registered SODirectorComponent."), *GetNameSafe(TargetObject));
		return;
	}

	const USmartObjectDefinition* Definition = Director->GetDefinition();
	if (!Definition)
	{
		return;
	}

	const FSmartObjectHandle ObjectHandle = Director->GetRegisteredHandle();

	TArray<FSmartObjectSlotHandle> AllSlots;
	Subsystem->GetAllSlots(ObjectHandle, AllSlots);

	const TConstArrayView<FSmartObjectSlotDefinition> SlotDefinitions = Definition->GetSlots();

	// Reserve the first claimable slot whose activity tags match SlotTag (or the first free slot if SlotTag is unset).
	for (const FSmartObjectSlotHandle& SlotHandle : AllSlots)
	{
		const int32 SlotIndex = SlotHandle.GetSlotIndex();
		if (!SlotDefinitions.IsValidIndex(SlotIndex))
		{
			continue;
		}

		if (SlotTag.IsValid() && !SlotDefinitions[SlotIndex].ActivityTags.HasTag(SlotTag))
		{
			continue;
		}

		if (!Subsystem->CanBeClaimed(SlotHandle, ESmartObjectClaimPriority::Normal))
		{
			continue;
		}

		ClaimHandle = Subsystem->MarkSlotAsClaimed(
			SlotHandle,
			ESmartObjectClaimPriority::Normal,
			FConstStructView::Make(FSmartObjectActorUserData(Owner)));

		if (!ClaimHandle.IsValid())
		{
			continue;
		}

		ActiveSmartObject = TargetObject;

		// Align the interactor to the slot entry point.
		FSmartObjectSlotEntranceLocationRequest EntranceRequest;
		EntranceRequest.UserActor = Owner;
		EntranceRequest.SearchLocation = Owner->GetActorLocation();
		EntranceRequest.bUseSlotLocationAsFallback = true;

		FSmartObjectSlotEntranceLocationResult Entrance;
		if (Subsystem->FindEntranceLocationForSlot(SlotHandle, EntranceRequest, Entrance))
		{
			SyncWarpTranslationAndRotation(Entrance.Location, Entrance.Rotation);
		}
		else if (const TOptional<FTransform> SlotTransform = Subsystem->GetSlotTransform(SlotHandle))
		{
			SyncWarpTranslationAndRotation(SlotTransform->GetLocation(), SlotTransform->Rotator());
		}

		UE_LOG(LogSmartObjectDirector, Verbose, TEXT("ServerClaimAndInteract: %s claimed slot %s on %s."),
			*GetNameSafe(Owner), *LexToString(SlotHandle), *GetNameSafe(TargetObject));
		return;
	}

	UE_LOG(LogSmartObjectDirector, Verbose, TEXT("ServerClaimAndInteract: no claimable slot found on %s."), *GetNameSafe(TargetObject));
}

void USOInteractionComponent::CancelCurrentInteraction()
{
	// Reservation state lives on the authority; ensure it is the one to release it.
	if (GetOwnerRole() == ROLE_Authority)
	{
		ReleaseClaim();
	}
}

void USOInteractionComponent::ReleaseClaim()
{
	if (ClaimHandle.IsValid())
	{
		if (USmartObjectSubsystem* Subsystem = GetSubsystem())
		{
			Subsystem->MarkSlotAsFree(ClaimHandle);
		}
		ClaimHandle.Invalidate();
	}

	ActiveSmartObject = nullptr;
}

void USOInteractionComponent::SyncWarpTranslationAndRotation(FVector TargetLocation, FRotator TargetRotation)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Preferred path: drive a Motion Warping target so the interaction montage aligns naturally.
	if (UMotionWarpingComponent* WarpComponent = Owner->FindComponentByClass<UMotionWarpingComponent>())
	{
		WarpComponent->AddOrUpdateWarpTargetFromLocationAndRotation(MotionWarpTargetName, TargetLocation, TargetRotation);
		return;
	}

	// Graceful fallback when no Motion Warping component exists: teleport + rotate to the entry point.
	Owner->SetActorLocationAndRotation(TargetLocation, TargetRotation, /*bSweep*/ false, /*OutSweepHitResult*/ nullptr, ETeleportType::TeleportPhysics);
}
