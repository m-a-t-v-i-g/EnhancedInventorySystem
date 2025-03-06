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

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	virtual void SetupInventoryManager(APawn* OwnPawn);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	virtual void ResetInventoryManager(APawn* OwnPawn);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	void AddReplicatedContainer(UEISItemContainer* Container);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	void RemoveReplicatedContainer(UEISItemContainer* Container);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	void AddReplicatedSlot(UEISEquipmentSlot* EquipmentSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	void RemoveReplicatedSlot(UEISEquipmentSlot* EquipmentSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Container")
	virtual void Container_AddItem(UObject* FromSource, UEISItemContainer* ToContainer, UEISItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Container")
	virtual void Container_RemoveItem(UEISItemContainer* Container, UEISItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Container")
	virtual void Container_StackItem(UObject* FromSource, UEISItemContainer* InContainer, UEISItemInstance* SourceItem,
	                                 UEISItemInstance* TargetItem);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Container")
	virtual void Container_SplitItem(UEISItemContainer* Container, UEISItemInstance* Item, int Amount);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Container")
	void MoveItemFromContainerToContainer(UEISItemContainer* FromContainer, UEISItemContainer* ToContainer,
										  UEISItemInstance* Item, bool bFullStack = false);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Container")
	void MoveItemFromContainerToSlot(UEISItemContainer* FromContainer, UEISEquipmentSlot* AtEquipmentSlot,
	                                 UEISItemInstance* Item);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Slot")
	void EquipSlot(UObject* FromSource, UEISEquipmentSlot* AtEquipmentSlot, UEISItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Slot")
	void UnequipSlot(UEISEquipmentSlot* EquipmentSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Slot")
	void MoveItemFromSlotToContainer(UEISEquipmentSlot* FromEquipmentSlot, UEISItemContainer* ToContainer);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Slot")
	void MoveItemFromSlotToSlot(UEISEquipmentSlot* FromEquipmentSlot, UEISEquipmentSlot* AtEquipmentSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Source")
	virtual void AddItemInSource(UObject* Source, UEISItemInstance* Item);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Source")
	virtual void LeaveItemInSource(UObject* Source, UEISItemInstance* Item, int Amount);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Source")
	virtual void RemoveItemFromSource(UObject* Source, UEISItemInstance* Item);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory Manager|Source")
	virtual void SubtractOrRemoveItemFromSource(UObject* Source, UEISItemInstance* Item, int Amount);

	UFUNCTION(BlueprintPure, Category = "Inventory Manager")
	UEISItemContainer* GetOwnItemContainer() const { return OwnItemContainer; }

	UFUNCTION(BlueprintPure, Category = "Inventory Manager")
	TArray<UEISEquipmentSlot*> GetOwnEquipmentSlots() const { return OwnEquipmentSlots; }

protected:
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnSetupInventoryManager")
	void K2_OnSetupInventoryManager(APawn* OwnPawn);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnResetInventoryManager")
	void K2_OnResetInventoryManager(APawn* OwnPawn);

	virtual void InternalAddItem(UObject* FromSource, UEISItemContainer* ToContainer, UEISItemInstance* Item);
	virtual void InternalRemoveItem(UEISItemContainer* Container, UEISItemInstance* Item);
	virtual void InternalStackItem(UObject* FromSource, UEISItemContainer* InContainer, UEISItemInstance* SourceItem,
	                               UEISItemInstance* TargetItem);
	virtual void InternalSplitItem(UEISItemContainer* Container, UEISItemInstance* Item, int Amount);
	virtual void InternalMoveItemFromContainerToContainer(UEISItemContainer* FromContainer,
	                                                      UEISItemContainer* ToContainer, UEISItemInstance* Item,
	                                                      bool bFullStack);
	virtual void InternalMoveItemFromContainerToSlot(UEISItemContainer* FromContainer,
	                                                 UEISEquipmentSlot* AtEquipmentSlot, UEISItemInstance* Item);
	
	virtual void InternalEquipSlot(UObject* FromSource, UEISEquipmentSlot* AtEquipmentSlot, UEISItemInstance* Item);
	virtual void InternalUnequipSlot(UEISEquipmentSlot* EquipmentSlot);
	virtual void InternalMoveItemFromSlotToContainer(UEISEquipmentSlot* FromEquipmentSlot, UEISItemContainer* ToContainer);
	virtual void InternalMoveItemFromSlotToSlot(UEISEquipmentSlot* FromEquipmentSlot, UEISEquipmentSlot* AtEquipmentSlot);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerContainerAddItem(UObject* FromSource, UEISItemContainer* ToContainer, UEISItemInstance* Item);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerContainerRemoveItem(UEISItemContainer* Container, UEISItemInstance* Item);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerContainerStackItem(UObject* FromSource, UEISItemContainer* InContainer, UEISItemInstance* SourceItem, UEISItemInstance* TargetItem);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerContainerSplitItem(UEISItemContainer* Container, UEISItemInstance* Item, int Amount);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerMoveItemFromContainerToContainer(UEISItemContainer* FromContainer, UEISItemContainer* ToContainer,
	                                            UEISItemInstance* Item, bool bFullStack);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerMoveItemFromContainerToSlot(UEISItemContainer* FromContainer, UEISEquipmentSlot* AtEquipmentSlot,
	                                       UEISItemInstance* Item);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSlotEquipItem(UObject* FromSource, UEISEquipmentSlot* AtEquipmentSlot, UEISItemInstance* Item);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSlotUnequipItem(UEISEquipmentSlot* EquipmentSlot);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerMoveItemFromSlotToContainer(UEISEquipmentSlot* FromEquipmentSlot, UEISItemContainer* ToContainer);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerMoveItemFromSlotToSlot(UEISEquipmentSlot* FromEquipmentSlot, UEISEquipmentSlot* AtEquipmentSlot);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Inventory Manager")
	bool bInitializeOnBeginPlay = false;

	UPROPERTY()
	TObjectPtr<UEISItemContainer> OwnItemContainer;

	UPROPERTY()
	TArray<UEISEquipmentSlot*> OwnEquipmentSlots;
	
	UPROPERTY(Replicated)
	FEISAppliedItemContainers ReplicatedContainers;
};
