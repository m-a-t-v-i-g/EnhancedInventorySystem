// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ControllerComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "EISInventoryManagerComponent.generated.h"

struct FEISAppliedItemContainers;
class UEISInventoryManagerComponent;
class UEISItemContainer;

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

	virtual void SetupInventoryManager();
	virtual void ResetInventoryManager();

	void AddReplicatedContainer(UEISItemContainer* Container);
	void RemoveReplicatedContainer(UEISItemContainer* Container);

protected:
	virtual void InitializeComponent() override final;
	virtual void UninitializeComponent() override final;
	
private:
	UPROPERTY(Replicated)
	FEISAppliedItemContainers ReplicatedContainers;
};
