// Copyright 2026 Simulated Flow All Rights Reserved.

#include "SOTask_UseSmartObject.h"
#include "SOInteractionComponent.h"
#include "SmartObjectDirectorLog.h"

#include "StateTreeExecutionContext.h"

#include "SmartObjectSubsystem.h"
#include "SmartObjectRequestTypes.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "MotionWarpingComponent.h"

#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

namespace
{
	/** Resolves the acting actor from the (optionally bound) instance data and StateTree owner. */
	AActor* ResolveInteractor(const FSOTask_UseSmartObjectInstanceData& InstanceData, UObject* Owner)
	{
		if (InstanceData.Interactor)
		{
			return InstanceData.Interactor;
		}

		if (AController* Controller = Cast<AController>(Owner))
		{
			return Controller->GetPawn();
		}

		return Cast<AActor>(Owner);
	}

	/** Returns the ability system component of an actor if it exposes one. */
	UAbilitySystemComponent* GetAbilitySystem(const AActor* Actor)
	{
		if (const IAbilitySystemInterface* Interface = Cast<const IAbilitySystemInterface>(Actor))
		{
			return Interface->GetAbilitySystemComponent();
		}
		return Actor ? Actor->FindComponentByClass<UAbilitySystemComponent>() : nullptr;
	}

	/** Warps/moves the interactor to the target transform, preferring Motion Warping. */
	void AlignInteractor(AActor* Interactor, const FVector& Location, const FRotator& Rotation)
	{
		if (!Interactor)
		{
			return;
		}

		// Prefer the interaction component's warp helper if present, so both paths share behavior.
		if (USOInteractionComponent* Interaction = Interactor->FindComponentByClass<USOInteractionComponent>())
		{
			Interaction->SyncWarpTranslationAndRotation(Location, Rotation);
			return;
		}

		if (UMotionWarpingComponent* WarpComponent = Interactor->FindComponentByClass<UMotionWarpingComponent>())
		{
			WarpComponent->AddOrUpdateWarpTargetFromLocationAndRotation(TEXT("SmartObjectAlign"), Location, Rotation);
			return;
		}

		Interactor->SetActorLocationAndRotation(Location, Rotation, /*bSweep*/ false, nullptr, ETeleportType::TeleportPhysics);
	}
}

FSOTask_UseSmartObject::FSOTask_UseSmartObject()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FSOTask_UseSmartObject::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.ElapsedTime = 0.0f;
	InstanceData.ClaimHandle.Invalidate();

	UWorld* World = Context.GetWorld();
	AActor* Interactor = ResolveInteractor(InstanceData, Context.GetOwner());
	if (!World || !Interactor)
	{
		return EStateTreeRunStatus::Failed;
	}

	USmartObjectSubsystem* Subsystem = USmartObjectSubsystem::GetCurrent(World);
	if (!Subsystem)
	{
		return EStateTreeRunStatus::Failed;
	}

	const FVector Origin = Interactor->GetActorLocation();
	const float Radius = FMath::Max(InstanceData.SearchRadius, 1.0f);

	FSmartObjectRequestFilter Filter;
	if (InstanceData.ActivityTag.IsValid())
	{
		Filter.ActivityRequirements = FGameplayTagQuery::MakeQuery_MatchAnyTags(FGameplayTagContainer(InstanceData.ActivityTag));
	}

	FSmartObjectRequest Request(FBox::BuildAABB(Origin, FVector(Radius)), Filter);

	const FSmartObjectRequestResult Result = Subsystem->FindSmartObject(Request, Interactor);
	if (!Result.IsValid())
	{
		UE_LOG(LogSmartObjectDirector, Verbose, TEXT("UseSmartObject task: no matching Smart Object near %s."), *GetNameSafe(Interactor));
		return EStateTreeRunStatus::Failed;
	}

	if (!Subsystem->CanBeClaimed(Result.SlotHandle, ESmartObjectClaimPriority::Normal))
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.ClaimHandle = Subsystem->MarkSlotAsClaimed(
		Result.SlotHandle,
		ESmartObjectClaimPriority::Normal,
		FConstStructView::Make(FSmartObjectActorUserData(Interactor)));

	if (!InstanceData.ClaimHandle.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	// Align the agent to the slot entry point.
	FSmartObjectSlotEntranceLocationRequest EntranceRequest;
	EntranceRequest.UserActor = Interactor;
	EntranceRequest.SearchLocation = Origin;
	EntranceRequest.bUseSlotLocationAsFallback = true;

	FSmartObjectSlotEntranceLocationResult Entrance;
	if (Subsystem->FindEntranceLocationForSlot(Result.SlotHandle, EntranceRequest, Entrance))
	{
		AlignInteractor(Interactor, Entrance.Location, Entrance.Rotation);
	}
	else if (const TOptional<FTransform> SlotTransform = Subsystem->GetSlotTransform(Result.SlotHandle))
	{
		AlignInteractor(Interactor, SlotTransform->GetLocation(), SlotTransform->Rotator());
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSOTask_UseSmartObject::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// Early completion when the interactor is granted the completion gameplay tag.
	if (InstanceData.CompletionTag.IsValid())
	{
		AActor* Interactor = ResolveInteractor(InstanceData, Context.GetOwner());
		if (const UAbilitySystemComponent* ASC = GetAbilitySystem(Interactor))
		{
			if (ASC->HasMatchingGameplayTag(InstanceData.CompletionTag))
			{
				return EStateTreeRunStatus::Succeeded;
			}
		}
	}

	InstanceData.ElapsedTime += DeltaTime;
	if (InstanceData.ElapsedTime >= InstanceData.InteractionDuration)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void FSOTask_UseSmartObject::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// Always release the reservation, whatever the reason we are leaving the state.
	if (InstanceData.ClaimHandle.IsValid())
	{
		if (UWorld* World = Context.GetWorld())
		{
			if (USmartObjectSubsystem* Subsystem = USmartObjectSubsystem::GetCurrent(World))
			{
				Subsystem->MarkSlotAsFree(InstanceData.ClaimHandle);
			}
		}
		InstanceData.ClaimHandle.Invalidate();
	}
}
