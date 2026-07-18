// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "SmartObjectTypes.h"
#include "SODirectorComponent.generated.h"

class USmartObjectDefinition;
class USmartObjectSubsystem;

/**
 * USODirectorComponent (Dynamic Registrator)
 *
 * Attach to any actor that should act as a dynamic Smart Object (runtime-spawned barricades,
 * crafting stations, vehicles, ...). Encapsulates a USmartObjectDefinition and registers a
 * runtime Smart Object instance with the global USmartObjectSubsystem, so slots can be enabled
 * or disabled at runtime from C++ or Blueprint without authoring a persistent collection.
 */
UCLASS(ClassGroup=(SmartObjectDirector), meta=(BlueprintSpawnableComponent))
class SMARTOBJECTDIRECTOR_API USODirectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USODirectorComponent();

	/** Registers the object with the Smart Object subsystem. Safe to call more than once. */
	UFUNCTION(BlueprintCallable, Category = "SmartObjectDirector")
	bool RegisterWithSubsystem();

	/** Removes the object from the Smart Object subsystem. */
	UFUNCTION(BlueprintCallable, Category = "SmartObjectDirector")
	void UnregisterFromSubsystem();

	/** Enables or disables every interaction slot whose activity tags contain SlotTag. Returns true if at least one slot matched. */
	UFUNCTION(BlueprintCallable, Category = "SmartObjectDirector")
	bool SetSlotEnabled(FGameplayTag SlotTag, bool bEnabled);

	/** Returns true if any slot matching SlotTag is currently claimed or occupied. */
	UFUNCTION(BlueprintPure, Category = "SmartObjectDirector")
	bool IsSlotClaimed(FGameplayTag SlotTag) const;

	/** Returns true while this component owns a valid runtime Smart Object handle. */
	UFUNCTION(BlueprintPure, Category = "SmartObjectDirector")
	bool IsRegistered() const { return RegisteredHandle.IsValid(); }

	/** Handle of the runtime Smart Object instance (invalid until registered). */
	FSmartObjectHandle GetRegisteredHandle() const { return RegisteredHandle; }

	/** Loads and returns the configured definition asset (synchronous load of the soft pointer). */
	const USmartObjectDefinition* GetDefinition() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Definition describing the slots/behaviors this dynamic Smart Object exposes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SmartObjectDirector")
	TSoftObjectPtr<USmartObjectDefinition> SmartObjectDefinitionAsset;

	/** If true, RegisterWithSubsystem() is called automatically on BeginPlay. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SmartObjectDirector")
	bool bAutoRegisterOnBeginPlay = true;

private:
	/** Resolves the Smart Object subsystem for this component's world. */
	USmartObjectSubsystem* GetSubsystem() const;

	/** Collects the runtime slot handles whose owning definition slot activity tags contain SlotTag. */
	void GatherSlotsForTag(FGameplayTag SlotTag, TArray<FSmartObjectSlotHandle>& OutSlots) const;

	/** Runtime handle of the created Smart Object instance. */
	FSmartObjectHandle RegisteredHandle;
};
