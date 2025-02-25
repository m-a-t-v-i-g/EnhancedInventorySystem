﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ControllerComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "EISInventoryManagerComponent.generated.h"

class UItemsContainer;
struct FEISAppliedItemContainers;
class UEISInventoryManagerComponent;
class UEISItemContainer;
class UEISItem;

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

UCLASS(DisplayName = "Inventory Manager Component", ClassGroup = "Enhanced Inventory System",
	meta = (BlueprintSpawnableComponent))
class ENHANCEDINVENTORYSYSTEM_API UEISInventoryManagerComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	UEISInventoryManagerComponent(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Manager")
	virtual void SetupInventoryManager(APawn* OwnPawn);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Manager")
	virtual void ResetInventoryManager();

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnSetupInventoryManager")
	void K2_OnSetupInventoryManager(APawn* OwnPawn);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnResetInventoryManager")
	void K2_OnResetInventoryManager();

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Manager")
	void AddReplicatedContainer(UEISItemContainer* Container);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Manager")
	void RemoveReplicatedContainer(UEISItemContainer* Container);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Manager|Container")
	void Container_AddItem(UObject* FromSource, UEISItemContainer* ToContainer, UEISItem* Item);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Manager|Container")
	void Container_RemoveItem(UEISItemContainer* Container, UEISItem* Item);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Manager|Container")
	void Container_StackItem(UObject* FromSource, UEISItemContainer* InContainer, UEISItem* SourceItem, UEISItem* TargetItem);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Manager|Container")
	void Container_SplitItem(UEISItemContainer* Container, UEISItem* Item, int Amount);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Manager|Container")
	void Container_MoveItemToOtherContainer(UEISItemContainer* FromContainer, UEISItemContainer* ToContainer,
	                                        UEISItem* Item);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Manager|Container")
	virtual void RemoveItemFromSource(UObject* Source, UEISItem* Item);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Inventory Manager")
	bool bInitializeOnBeginPlay = false;
	
	UPROPERTY(Replicated)
	FEISAppliedItemContainers ReplicatedContainers;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerContainerAddItem(UObject* FromSource, UEISItemContainer* ToContainer, UEISItem* Item);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerContainerRemoveItem(UEISItemContainer* Container, UEISItem* Item);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerContainerStackItem(UObject* FromSource, UEISItemContainer* InContainer, UEISItem* SourceItem, UEISItem* TargetItem);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSplitItem(UEISItemContainer* Container, UEISItem* Item, int Amount);

};
