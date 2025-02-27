// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ControllerComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "EISInventoryManagerComponent.generated.h"

struct FEISAppliedItemContainers;
class UEISInventoryManagerComponent;
class UEISItemContainer;
class UEISEquipmentSlot;
class UEISItemInstance;

USTRUCT()
struct FEISAppliedItemContainerEntry : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

	FEISAppliedItemContainerEntry()
	{
	}

private:
	friend UEISInventoryManagerComponent;
	friend FEISAppliedItemContainers;
	
	UPROPERTY()
	UEISItemContainer* ItemContainer = nullptr;
};

USTRUCT()
struct FEISAppliedItemContainers : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	FEISAppliedItemContainers()
	{
	}

	FEISAppliedItemContainers(UEISInventoryManagerComponent* InOwnerComponent) : InventoryManagerComponent(InOwnerComponent)
	{
	}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FEISAppliedItemContainerEntry, FEISAppliedItemContainers>(Entries, DeltaParams, *this);
	}

	UEISItemContainer* AddEntry(UEISItemContainer* ItemContainer);
	void RemoveEntry(UEISItemContainer* ItemContainer);
	void Clear();

private:
	friend UEISInventoryManagerComponent;
	
	UPROPERTY()
	TArray<FEISAppliedItemContainerEntry> Entries;

	UPROPERTY(NotReplicated)
	UEISInventoryManagerComponent* InventoryManagerComponent = nullptr;
};

UCLASS(DisplayName = "Inventory Manager Component", Abstract)
class ENHANCEDINVENTORYSYSTEM_API UEISInventoryManagerComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	UEISInventoryManagerComponent(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	virtual void SetupInventoryManager(APawn* OwnPawn);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	virtual void ResetInventoryManager(APawn* OwnPawn);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnSetupInventoryManager")
	void K2_OnSetupInventoryManager(APawn* OwnPawn);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnResetInventoryManager")
	void K2_OnResetInventoryManager();

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	void AddReplicatedContainer(UEISItemContainer* Container);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	void RemoveReplicatedContainer(UEISItemContainer* Container);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	void AddReplicatedSlot(UEISEquipmentSlot* EquipmentSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	void RemoveReplicatedSlot(UEISEquipmentSlot* EquipmentSlot);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Container")
	void Container_AddItem(UObject* FromSource, UEISItemContainer* ToContainer, UEISItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Container")
	void Container_RemoveItem(UEISItemContainer* Container, UEISItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Container")
	void Container_StackItem(UObject* FromSource, UEISItemContainer* InContainer, UEISItemInstance* SourceItem, UEISItemInstance* TargetItem);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Container")
	void Container_SplitItem(UEISItemContainer* Container, UEISItemInstance* Item, int Amount);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Slot")
	void Slot_TryEquipAny(UObject* Source, UEISItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Slot")
	void EquipSlot(UObject* FromSource, UEISEquipmentSlot* AtEquipmentSlot, UEISItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Slot")
	void UnequipSlot(UEISEquipmentSlot* EquipmentSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Container")
	void MoveItemFromContainerToContainer(UEISItemContainer* FromContainer, UEISItemContainer* ToContainer,
	                                      UEISItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Slot")
	void MoveItemFromSlotToContainer(UEISEquipmentSlot* SourceSlot, UEISItemContainer* TargetContainer);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Container")
	virtual void RemoveItemFromSource(UObject* Source, UEISItemInstance* Item);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Inventory Manager")
	bool bInitializeOnBeginPlay = false;
	
	UPROPERTY(Replicated)
	FEISAppliedItemContainers ReplicatedContainers;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerContainerAddItem(UObject* FromSource, UEISItemContainer* ToContainer, UEISItemInstance* Item);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerContainerRemoveItem(UEISItemContainer* Container, UEISItemInstance* Item);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerContainerStackItem(UObject* FromSource, UEISItemContainer* InContainer, UEISItemInstance* SourceItem, UEISItemInstance* TargetItem);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSplitItem(UEISItemContainer* Container, UEISItemInstance* Item, int Amount);
};
