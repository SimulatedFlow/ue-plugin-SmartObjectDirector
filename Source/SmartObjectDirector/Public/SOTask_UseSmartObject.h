// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "SmartObjectRuntime.h"
#include "SmartObjectTypes.h"
#include "SOTask_UseSmartObject.generated.h"

class AActor;

/**
 * Instance data for FSOTask_UseSmartObject.
 * The Interactor and query parameters are authored/bound in the StateTree editor; the remaining
 * fields hold the per-execution runtime state (reservation handle and elapsed interaction time).
 */
USTRUCT()
struct FSOTask_UseSmartObjectInstanceData
{
	GENERATED_BODY()

	/** Agent that will use the Smart Object. If unset, the StateTree owner (or its pawn) is used. */
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Interactor = nullptr;

	/** Only slots whose activity tags contain this tag are considered. Unset = any slot. */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag ActivityTag;

	/** Search radius (cm) around the interactor used to find a Smart Object slot. */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float SearchRadius = 1500.0f;

	/**
	 * Maximum time (seconds) to hold the interaction before the task auto-succeeds.
	 * If CompletionTag is set and present on the interactor's ability system, the task
	 * succeeds as soon as the tag is granted (whichever happens first).
	 */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float InteractionDuration = 3.0f;

	/** Optional gameplay tag which, once present on the interactor, completes the interaction early. */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag CompletionTag;

	/** Runtime: elapsed interaction time. */
	UPROPERTY()
	float ElapsedTime = 0.0f;

	/** Runtime: active reservation for the claimed slot. */
	UPROPERTY()
	FSmartObjectClaimHandle ClaimHandle;
};

/**
 * FSOTask_UseSmartObject
 *
 * StateTree task that lets an AI agent natively use a Smart Object: it searches for a slot with
 * the requested activity tag, reserves it, warps/moves the agent to the entry point, triggers the
 * interaction and waits either for the interaction duration to elapse or for a completion gameplay
 * tag to be granted. The reservation is always released on exit.
 */
USTRUCT(meta = (DisplayName = "Use Smart Object (Director)", Category = "SmartObjectDirector"))
struct SMARTOBJECTDIRECTOR_API FSOTask_UseSmartObject : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSOTask_UseSmartObjectInstanceData;

	FSOTask_UseSmartObject();

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
