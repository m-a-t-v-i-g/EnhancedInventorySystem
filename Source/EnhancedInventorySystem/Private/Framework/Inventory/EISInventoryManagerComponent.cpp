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

void UEISInventoryManagerComponent::SetupInventoryManager(APawn* OwnPawn)
{
	if (OwnPawn != nullptr)
	{
		auto InventoryComponent = OwnPawn->GetComponentByClass<UEISInventoryComponent>();
		if (!InventoryComponent)
		{
			return;
		}

		if (UEISItemContainer* ItemsContainer = InventoryComponent->GetItemContainer())
		{
			if (HasAuthority())
			{
				AddReplicatedContainer(ItemsContainer);
			}
		}
	}
	
	K2_OnSetupInventoryManager(OwnPawn);
}

void UEISInventoryManagerComponent::ResetInventoryManager()
{
	ReplicatedContainers.Clear();

	K2_OnResetInventoryManager();
}

void UEISInventoryManagerComponent::BeginPlay()
{
	if (bInitializeOnBeginPlay)
	{
		SetupInventoryManager(GetPawn<APawn>());
	}
	
	Super::BeginPlay();
}

void UEISInventoryManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ResetInventoryManager();
	
	Super::EndPlay(EndPlayReason);
}

void UEISInventoryManagerComponent::Container_AddItem(UObject* FromSource, UEISItemContainer* ToContainer,
                                                      UEISItemInstance* Item)
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

	ServerContainerAddItem(FromSource, ToContainer, Item);
}

void UEISInventoryManagerComponent::Container_RemoveItem(UEISItemContainer* Container, UEISItemInstance* Item)
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

	ServerContainerRemoveItem(Container, Item);
}

void UEISInventoryManagerComponent::Container_StackItem(UObject* FromSource, UEISItemContainer* InContainer,
                                                        UEISItemInstance* SourceItem, UEISItemInstance* TargetItem)
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

	ServerContainerStackItem(FromSource, InContainer, SourceItem, TargetItem);
}

void UEISInventoryManagerComponent::Container_SplitItem(UEISItemContainer* Container, UEISItemInstance* Item, int Amount)
{
	check(Container);
	check(Item);

	if (!GetController<AController>() || Amount <= 0)
	{
		return;
	}
	
	if (!HasAuthority() && IsLocalController())
	{
		if (Container->CanAddItem(Item))
		{
			if (Item->GetAmount() > 1 && Item->GetAmount() > Amount)
			{
				Item->RemoveAmount(Amount);
			}
		}
	}

	ServerSplitItem(Container, Item, Amount);
}

void UEISInventoryManagerComponent::Container_MoveItemToOtherContainer(UEISItemContainer* FromContainer,
                                                                       UEISItemContainer* ToContainer, UEISItemInstance* Item)
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

void UEISInventoryManagerComponent::RemoveItemFromSource(UObject* Source, UEISItemInstance* Item)
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

void UEISInventoryManagerComponent::ServerContainerAddItem_Implementation(UObject* FromSource, UEISItemContainer* ToContainer, UEISItemInstance* Item)
{
	UEISInventoryFunctionLibrary::Container_AddItem(ToContainer, Item);
	RemoveItemFromSource(FromSource, Item);
}

bool UEISInventoryManagerComponent::ServerContainerAddItem_Validate(UObject* FromSource, UEISItemContainer* ToContainer, UEISItemInstance* Item)
{
	return IsValid(FromSource) && IsValid(ToContainer) && IsValid(Item);
}

void UEISInventoryManagerComponent::ServerContainerRemoveItem_Implementation(UEISItemContainer* Container, UEISItemInstance* Item)
{
	UEISInventoryFunctionLibrary::Container_RemoveItem(Container, Item);
}

bool UEISInventoryManagerComponent::ServerContainerRemoveItem_Validate(UEISItemContainer* Container, UEISItemInstance* Item)
{
	return IsValid(Container) && IsValid(Item);
}

void UEISInventoryManagerComponent::ServerContainerStackItem_Implementation(
	UObject* FromSource, UEISItemContainer* InContainer, UEISItemInstance* SourceItem, UEISItemInstance* TargetItem)
{
	UEISInventoryFunctionLibrary::Container_StackItem(InContainer, SourceItem, TargetItem);
	RemoveItemFromSource(FromSource, SourceItem);
}

bool UEISInventoryManagerComponent::ServerContainerStackItem_Validate(UObject* FromSource,
                                                                      UEISItemContainer* InContainer,
                                                                      UEISItemInstance* SourceItem, UEISItemInstance* TargetItem)
{
	return IsValid(FromSource) && IsValid(InContainer) && IsValid(SourceItem) && IsValid(TargetItem);
}

void UEISInventoryManagerComponent::ServerSplitItem_Implementation(UEISItemContainer* Container, UEISItemInstance* Item, int Amount)
{
	UEISInventoryFunctionLibrary::Container_SplitItem(Container, Item, Amount);
}

bool UEISInventoryManagerComponent::ServerSplitItem_Validate(UEISItemContainer* Container, UEISItemInstance* Item, int Amount)
{
	return IsValid(Container) && IsValid(Item) && Amount > 0;
}
