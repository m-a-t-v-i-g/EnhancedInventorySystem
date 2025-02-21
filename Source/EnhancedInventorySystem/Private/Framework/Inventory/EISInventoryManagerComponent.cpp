// Fill out your copyright notice in the Description page of Project Settings.

#include "EISInventoryManagerComponent.h"
#include "EISInventoryComponent.h"
#include "EISItemContainer.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

UEISItemContainer* FEISAppliedItemContainers::AddEntry(UEISItemContainer* ItemContainer)
{
	check(ItemContainer);
	
	FEISAppliedItemContainerEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.ItemContainer = ItemContainer;

	MarkItemDirty(NewEntry);
	return NewEntry.ItemContainer;
}

void FEISAppliedItemContainers::RemoveEntry(UEISItemContainer* ItemContainer)
{
	check(ItemContainer);
	
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FEISAppliedItemContainerEntry& Entry = *EntryIt;
		if (Entry.ItemContainer == ItemContainer)
		{
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}

void FEISAppliedItemContainers::Clear()
{
	Entries.Empty();
}

UEISInventoryManagerComponent::UEISInventoryManagerComponent(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer), ReplicatedContainers(this)
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;

	SetIsReplicatedByDefault(true);
}

void UEISInventoryManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, ReplicatedContainers, COND_OwnerOnly);
}

bool UEISInventoryManagerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
                                                        FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (FEISAppliedItemContainerEntry& Entry : ReplicatedContainers.Entries)
	{
		UEISItemContainer* Instance = Entry.ItemContainer;
		if (IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

void UEISInventoryManagerComponent::SetupInventoryManager()
{
	auto Pawn = GetPawn<APawn>();
	if (Pawn != nullptr)
	{
		if (auto InventoryComponent = Pawn->GetComponentByClass<UEISInventoryComponent>())
		{
			if (UEISItemContainer* ItemsContainer = InventoryComponent->GetItemContainer())
			{
				if (HasAuthority())
				{
					AddReplicatedContainer(ItemsContainer);
				}
			}
		}
	}
}

void UEISInventoryManagerComponent::ResetInventoryManager()
{
	ReplicatedContainers.Clear();
}

void UEISInventoryManagerComponent::AddReplicatedContainer(UEISItemContainer* Container)
{
	ReplicatedContainers.AddEntry(Container);
}

void UEISInventoryManagerComponent::RemoveReplicatedContainer(UEISItemContainer* Container)
{
	ReplicatedContainers.RemoveEntry(Container);
}

void UEISInventoryManagerComponent::InitializeComponent()
{
	SetupInventoryManager();
	Super::InitializeComponent();
}

void UEISInventoryManagerComponent::UninitializeComponent()
{
	ResetInventoryManager();
	Super::UninitializeComponent();
}
