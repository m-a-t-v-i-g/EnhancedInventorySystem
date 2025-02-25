// Fill out your copyright notice in the Description page of Project Settings.

#include "EISInventoryManagerComponent.h"
#include "EISInventoryComponent.h"
#include "EISInventoryFunctionLibrary.h"
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

void UEISInventoryManagerComponent::AddReplicatedContainer(UEISItemContainer* Container)
{
	ReplicatedContainers.AddEntry(Container);
}

void UEISInventoryManagerComponent::RemoveReplicatedContainer(UEISItemContainer* Container)
{
	ReplicatedContainers.RemoveEntry(Container);
}

void UEISInventoryManagerComponent::BeginPlay()
{
	SetupInventoryManager();
	
	Super::BeginPlay();
}

void UEISInventoryManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ResetInventoryManager();
	
	Super::EndPlay(EndPlayReason);
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

	K2_OnSetupInventoryManager();
}

void UEISInventoryManagerComponent::ResetInventoryManager()
{
	ReplicatedContainers.Clear();

	K2_OnResetInventoryManager();
}

void UEISInventoryManagerComponent::Container_AddItem(UObject* FromSource, UEISItemContainer* ToContainer,
                                                      UEISItem* Item)
{
	check(FromSource);
	check(ToContainer);
	check(Item);

	if (!GetController<AController>())
	{
		return;
	}
	
	if (!HasAuthority() && IsLocalController())
	{
		UEISInventoryFunctionLibrary::Container_AddItem(ToContainer, Item);
		RemoveItemFromSource(FromSource, Item);
	}
}

void UEISInventoryManagerComponent::Container_RemoveItem(UEISItemContainer* Container, UEISItem* Item)
{
	check(Container);
	check(Item);

	if (!GetController<AController>())
	{
		return;
	}
	
	if (!HasAuthority() && IsLocalController())
	{
		UEISInventoryFunctionLibrary::Container_RemoveItem(Container, Item);
	}
}

void UEISInventoryManagerComponent::Container_StackItem(UObject* FromSource, UEISItemContainer* InContainer,
                                                        UEISItem* SourceItem, UEISItem* TargetItem)
{
	check(FromSource);
	check(InContainer);
	check(SourceItem);
	check(TargetItem);
	
	if (!GetController<AController>())
	{
		return;
	}
	
	if (!HasAuthority() && IsLocalController())
	{
		UEISInventoryFunctionLibrary::Container_StackItem(InContainer, SourceItem, TargetItem);
		RemoveItemFromSource(FromSource, SourceItem);
	}
}

void UEISInventoryManagerComponent::Container_SplitItem(UEISItemContainer* Container, UEISItem* Item, int Amount)
{
	check(Container);
	check(Item);

	if (!GetController<AController>() || Amount <= 0)
	{
		return;
	}
	
	if (!HasAuthority() && IsLocalController())
	{
		UEISInventoryFunctionLibrary::Container_SplitItem(Container, Item, Amount);
	}
}

void UEISInventoryManagerComponent::Container_MoveItemToOtherContainer(UEISItemContainer* FromContainer,
                                                                       UEISItemContainer* ToContainer, UEISItem* Item)
{
	check(FromContainer);
	check(ToContainer);
	check(Item);

	if (!GetController<AController>())
	{
		return;
	}

	if (!HasAuthority() && IsLocalController())
	{
		UEISInventoryFunctionLibrary::Container_MoveItemToOtherContainer(FromContainer, ToContainer, Item);
	}
}

void UEISInventoryManagerComponent::RemoveItemFromSource(UObject* Source, UEISItem* Item)
{
	if (auto SourceContainer = Cast<UEISItemContainer>(Source))
	{
		if (SourceContainer->Contains(Item))
		{
			UEISInventoryFunctionLibrary::Container_RemoveItem(SourceContainer, Item);
		}
		return;
	}
}
