// Copyright 2026 Silvan Teufel All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "SmartObjectRuntime.h"
#include "SmartObjectTypes.h"
#include "SOInteractionComponent.generated.h"

class USmartObjectSubsystem;

/**
 * USOInteractionComponent (Player & AI Bridge)
 *
 * Attach to any pawn/character (player- or AI-controlled). Finds nearby Smart Object Director
 * instances, validates them against filter tags, reserves a slot server-authoritatively (to
 * prevent multiplayer claim race-conditions), warps the character to the slot entry point via
 * Motion Warping (with a graceful teleport/rotation fallback), and releases the slot cleanly.
 */
UCLASS(ClassGroup=(SmartObjectDirector), meta=(BlueprintSpawnableComponent))
class SMARTOBJECTDIRECTOR_API USOInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USOInteractionComponent();

	/** Finds the best matching Smart Object owner actor within QueryRadius that satisfies FilterTags. */
	UFUNCTION(BlueprintCallable, Category = "SmartObjectDirector")
	AActor* FindBestInteractable(float QueryRadius, FGameplayTagContainer FilterTags);

	/** Reserves a slot on the server (authoritative), then warps and starts the interaction. */
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "SmartObjectDirector")
	void ServerClaimAndInteract(AActor* TargetObject, FGameplayTag SlotTag);

	/** Cleanly aborts the current interaction and frees the reserved slot (server-authoritative). */
	UFUNCTION(BlueprintCallable, Category = "SmartObjectDirector")
	void CancelCurrentInteraction();

	/** Returns true while a slot is reserved and an interaction is active. */
	UFUNCTION(BlueprintPure, Category = "SmartObjectDirector")
	bool IsInteracting() const { return ActiveSmartObject != nullptr; }

	/** Applies Motion Warping alignment to the slot entry point, falling back to teleport/rotate. */
	void SyncWarpTranslationAndRotation(FVector TargetLocation, FRotator TargetRotation);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Smart Object actor currently being interacted with (replicated to owning client). */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "SmartObjectDirector")
	TObjectPtr<AActor> ActiveSmartObject = nullptr;

	/** Motion Warping target name driven by the interaction montage. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SmartObjectDirector")
	FName MotionWarpTargetName = TEXT("SmartObjectAlign");

private:
	/** Resolves the Smart Object subsystem for this component's world. */
	USmartObjectSubsystem* GetSubsystem() const;

	/** Server-side: releases any reserved slot and clears interaction state. */
	void ReleaseClaim();

	/** Active reservation (only valid on the authority). */
	FSmartObjectClaimHandle ClaimHandle;
};
