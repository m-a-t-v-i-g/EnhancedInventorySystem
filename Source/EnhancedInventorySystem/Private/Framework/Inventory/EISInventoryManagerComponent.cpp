// Fill out your copyright notice in the Description page of Project Settings.

#include "EISInventoryManagerComponent.h"
#include "EISEquipmentComponent.h"
#include "EISEquipmentSlot.h"
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
	ResetInventoryManager(GetPawn<APawn>());
	
	Super::EndPlay(EndPlayReason);
}

void UEISInventoryManagerComponent::AddReplicatedContainer(UEISItemContainer* Container)
{
	ReplicatedContainers.AddEntry(Container);
}

void UEISInventoryManagerComponent::RemoveReplicatedContainer(UEISItemContainer* Container)
{
	ReplicatedContainers.RemoveEntry(Container);
}

void UEISInventoryManagerComponent::AddReplicatedSlot(UEISEquipmentSlot* EquipmentSlot)
{
	
}

void UEISInventoryManagerComponent::RemoveReplicatedSlot(UEISEquipmentSlot* EquipmentSlot)
{
	
}

void UEISInventoryManagerComponent::SetupInventoryManager(APawn* OwnPawn)
{
	if (OwnPawn != nullptr)
	{
		auto InventoryComponent = OwnPawn->GetComponentByClass<UEISInventoryComponent>();
		if (InventoryComponent != nullptr)
		{
			if (UEISItemContainer* ItemsContainer = InventoryComponent->GetItemContainer())
			{
				OwnItemContainer = ItemsContainer;
				
				if (HasAuthority())
				{
					AddReplicatedContainer(ItemsContainer);
				}
			}
		}

		auto EquipmentComponent = OwnPawn->GetComponentByClass<UEISEquipmentComponent>();
		if (EquipmentComponent != nullptr)
		{
			TArray<UEISEquipmentSlot*> Slots = EquipmentComponent->GetEquipmentSlots();
			if (!Slots.IsEmpty())
			{
				OwnEquipmentSlots = Slots;
				
				if (HasAuthority())
				{
					for (UEISEquipmentSlot* Slot : Slots)
					{
						AddReplicatedSlot(Slot);
					}
				}
			}
		}
	}
	
	K2_OnSetupInventoryManager(OwnPawn);
}

void UEISInventoryManagerComponent::ResetInventoryManager(APawn* OwnPawn)
{
	K2_OnResetInventoryManager(OwnPawn);
	
	ReplicatedContainers.Clear();
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
		ServerContainerAddItem(FromSource, ToContainer, Item);
	}

	InternalAddItem(FromSource, ToContainer, Item);
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
		ServerContainerRemoveItem(Container, Item);
	}

	InternalRemoveItem(Container, Item);
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
		ServerContainerStackItem(FromSource, InContainer, SourceItem, TargetItem);
	}

	InternalStackItem(FromSource, InContainer, SourceItem, TargetItem);
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
		ServerContainerSplitItem(Container, Item, Amount);
	}

	InternalSplitItem(Container, Item, Amount);
}

void UEISInventoryManagerComponent::MoveItemFromContainerToContainer(UEISItemContainer* FromContainer,
																	 UEISItemContainer* ToContainer,
																	 UEISItemInstance* Item, bool bFullStack)
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
		ServerMoveItemFromContainerToContainer(FromContainer, ToContainer, Item, bFullStack);
	}
	
	InternalMoveItemFromContainerToContainer(FromContainer, ToContainer, Item, bFullStack);
}

void UEISInventoryManagerComponent::MoveItemFromContainerToSlot(UEISItemContainer* FromContainer,
																UEISEquipmentSlot* AtEquipmentSlot,
																UEISItemInstance* Item)
{
	check(FromContainer);
	check(AtEquipmentSlot);
	check(Item);

	if (!GetController<AController>())
	{
		return;
	}

	if (!HasAuthority() && IsLocalController())
	{
		ServerMoveItemFromContainerToSlot(FromContainer, AtEquipmentSlot, Item);
	}
	
	InternalMoveItemFromContainerToSlot(FromContainer, AtEquipmentSlot, Item);
}

void UEISInventoryManagerComponent::EquipSlot(UObject* FromSource, UEISEquipmentSlot* AtEquipmentSlot,
                                              UEISItemInstance* Item)
{
	check(FromSource);
	check(AtEquipmentSlot);
	check(Item);

	if (!GetController<AController>())
	{
		return;
	}

	if (!HasAuthority() && IsLocalController())
	{
		ServerSlotEquipItem(FromSource, AtEquipmentSlot, Item);
	}

	InternalEquipSlot(FromSource, AtEquipmentSlot, Item);
}

void UEISInventoryManagerComponent::UnequipSlot(UEISEquipmentSlot* EquipmentSlot)
{
	check(EquipmentSlot);

	if (!GetController<AController>())
	{
		return;
	}

	if (!HasAuthority() && IsLocalController())
	{
		ServerSlotUnequipItem(EquipmentSlot);
	}

	InternalUnequipSlot(EquipmentSlot);
}

void UEISInventoryManagerComponent::MoveItemFromSlotToContainer(UEISEquipmentSlot* FromEquipmentSlot, UEISItemContainer* ToContainer)
{
	check(FromEquipmentSlot);
	check(ToContainer);
	
	if (!GetController<AController>())
	{
		return;
	}

	if (!HasAuthority() && IsLocalController())
	{
		ServerMoveItemFromSlotToContainer(FromEquipmentSlot, ToContainer);
	}

	InternalMoveItemFromSlotToContainer(FromEquipmentSlot, ToContainer);
}

void UEISInventoryManagerComponent::MoveItemFromSlotToSlot(UEISEquipmentSlot* FromEquipmentSlot,
                                                           UEISEquipmentSlot* AtEquipmentSlot)
{
	check(FromEquipmentSlot);
	check(AtEquipmentSlot);
	
	if (!GetController<AController>())
	{
		return;
	}

	if (!HasAuthority() && IsLocalController())
	{
		ServerMoveItemFromSlotToSlot(FromEquipmentSlot, AtEquipmentSlot);
	}

	InternalMoveItemFromSlotToSlot(FromEquipmentSlot, AtEquipmentSlot);
}

void UEISInventoryManagerComponent::AddItemInSource(UObject* Source, UEISItemInstance* Item)
{
	UEISInventoryFunctionLibrary::AddItemInSource(Source, Item);
}

void UEISInventoryManagerComponent::LeaveItemInSource(UObject* Source, UEISItemInstance* Item, int Amount)
{
	UEISItemInstance* RemainItem = UEISInventoryFunctionLibrary::GenerateItem(GetWorld(), Item);
	if (RemainItem != nullptr)
	{
		RemainItem->SetAmount(Amount);
	}
	
	UEISInventoryFunctionLibrary::LeaveItemInSource(Source, RemainItem);
}

void UEISInventoryManagerComponent::RemoveItemFromSource(UObject* Source, UEISItemInstance* Item)
{
	UEISInventoryFunctionLibrary::RemoveItemFromSource(Source, Item);
}

void UEISInventoryManagerComponent::SubtractOrRemoveItemFromSource(UObject* Source, UEISItemInstance* Item, int Amount)
{
	UEISInventoryFunctionLibrary::SubtractOrRemoveItemFromSource(Source, Item, Amount);
}

void UEISInventoryManagerComponent::InternalAddItem(UObject* FromSource, UEISItemContainer* ToContainer, UEISItemInstance* Item)
{
	UEISInventoryFunctionLibrary::Container_AddItem(ToContainer, Item);
	RemoveItemFromSource(FromSource, Item);
}

void UEISInventoryManagerComponent::InternalRemoveItem(UEISItemContainer* Container, UEISItemInstance* Item)
{
	UEISInventoryFunctionLibrary::Container_RemoveItem(Container, Item);
}

void UEISInventoryManagerComponent::InternalStackItem(UObject* FromSource, UEISItemContainer* InContainer,
                                                      UEISItemInstance* SourceItem, UEISItemInstance* TargetItem)
{
	UEISInventoryFunctionLibrary::Container_StackItem(InContainer, SourceItem, TargetItem);
	RemoveItemFromSource(FromSource, SourceItem);
}

void UEISInventoryManagerComponent::InternalSplitItem(UEISItemContainer* Container, UEISItemInstance* Item, int Amount)
{
	if (HasAuthority())
	{
		UEISInventoryFunctionLibrary::Container_SplitItem(Container, Item, Amount);
	}
	else if (!HasAuthority() && IsLocalController())
	{
		if (Item->GetAmount() > 1 && Item->GetAmount() > Amount)
		{
			Item->RemoveAmount(Amount);
		}
	}
}

void UEISInventoryManagerComponent::InternalMoveItemFromContainerToContainer(
	UEISItemContainer* FromContainer, UEISItemContainer* ToContainer, UEISItemInstance* Item, bool bFullStack)
{
	UEISInventoryFunctionLibrary::MoveItemFromContainerToContainer(FromContainer, ToContainer, Item, bFullStack);
}

void UEISInventoryManagerComponent::InternalMoveItemFromContainerToSlot(UEISItemContainer* FromContainer,
                                                                        UEISEquipmentSlot* AtEquipmentSlot,
                                                                        UEISItemInstance* Item)
{
	UEISInventoryFunctionLibrary::MoveItemFromContainerToSlot(FromContainer, AtEquipmentSlot, Item);
}

void UEISInventoryManagerComponent::InternalEquipSlot(UObject* FromSource, UEISEquipmentSlot* AtEquipmentSlot,
                                                      UEISItemInstance* Item)
{
	UEISInventoryFunctionLibrary::Slot_EquipItem(AtEquipmentSlot, Item);
	RemoveItemFromSource(FromSource, Item);
}

void UEISInventoryManagerComponent::InternalUnequipSlot(UEISEquipmentSlot* EquipmentSlot)
{
	UEISInventoryFunctionLibrary::Slot_UnequipItem(EquipmentSlot);
}

void UEISInventoryManagerComponent::InternalMoveItemFromSlotToContainer(UEISEquipmentSlot* FromEquipmentSlot,
                                                                        UEISItemContainer* ToContainer)
{
	UEISInventoryFunctionLibrary::MoveItemFromSlotToContainer(FromEquipmentSlot, ToContainer);
}

void UEISInventoryManagerComponent::InternalMoveItemFromSlotToSlot(UEISEquipmentSlot* FromEquipmentSlot,
                                                                   UEISEquipmentSlot* AtEquipmentSlot)
{
	UEISInventoryFunctionLibrary::MoveItemFromSlotToSlot(FromEquipmentSlot, AtEquipmentSlot);
}

void UEISInventoryManagerComponent::ServerContainerAddItem_Implementation(UObject* FromSource, UEISItemContainer* ToContainer, UEISItemInstance* Item)
{
	InternalAddItem(FromSource, ToContainer, Item);
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

void UEISInventoryManagerComponent::ServerContainerSplitItem_Implementation(UEISItemContainer* Container, UEISItemInstance* Item, int Amount)
{
	InternalSplitItem(Container, Item, Amount);
}

bool UEISInventoryManagerComponent::ServerContainerSplitItem_Validate(UEISItemContainer* Container, UEISItemInstance* Item, int Amount)
{
	return IsValid(Container) && IsValid(Item) && Amount > 0;
}

void UEISInventoryManagerComponent::ServerMoveItemFromContainerToContainer_Implementation(
	UEISItemContainer* FromContainer, UEISItemContainer* ToContainer, UEISItemInstance* Item, bool bFullStack)
{
	InternalMoveItemFromContainerToContainer(FromContainer, ToContainer, Item, bFullStack);
}

bool UEISInventoryManagerComponent::ServerMoveItemFromContainerToContainer_Validate(
	UEISItemContainer* FromContainer, UEISItemContainer* ToContainer, UEISItemInstance* Item, bool bFullStack)
{
	return IsValid(FromContainer) && IsValid(ToContainer) && IsValid(Item);
}

void UEISInventoryManagerComponent::ServerMoveItemFromContainerToSlot_Implementation(
	UEISItemContainer* FromContainer, UEISEquipmentSlot* AtEquipmentSlot, UEISItemInstance* Item)
{
	InternalMoveItemFromContainerToSlot(FromContainer, AtEquipmentSlot, Item);
}

bool UEISInventoryManagerComponent::ServerMoveItemFromContainerToSlot_Validate(
	UEISItemContainer* FromContainer, UEISEquipmentSlot* AtEquipmentSlot, UEISItemInstance* Item)
{
	return IsValid(FromContainer) && IsValid(AtEquipmentSlot) && IsValid(Item);
}

void UEISInventoryManagerComponent::ServerSlotEquipItem_Implementation(UObject* FromSource,
                                                                       UEISEquipmentSlot* AtEquipmentSlot,
                                                                       UEISItemInstance* Item)
{
	InternalEquipSlot(FromSource, AtEquipmentSlot, Item);
}

bool UEISInventoryManagerComponent::ServerSlotEquipItem_Validate(UObject* FromSource,
                                                                 UEISEquipmentSlot* AtEquipmentSlot,
                                                                 UEISItemInstance* Item)
{
	return IsValid(FromSource) && IsValid(AtEquipmentSlot) && IsValid(Item);
}

void UEISInventoryManagerComponent::ServerSlotUnequipItem_Implementation(UEISEquipmentSlot* EquipmentSlot)
{
	UEISInventoryFunctionLibrary::Slot_UnequipItem(EquipmentSlot);
}

bool UEISInventoryManagerComponent::ServerSlotUnequipItem_Validate(UEISEquipmentSlot* EquipmentSlot)
{
	return IsValid(EquipmentSlot);
}

void UEISInventoryManagerComponent::ServerMoveItemFromSlotToContainer_Implementation(
	UEISEquipmentSlot* FromEquipmentSlot, UEISItemContainer* ToContainer)
{
	InternalMoveItemFromSlotToContainer(FromEquipmentSlot, ToContainer);
}

bool UEISInventoryManagerComponent::ServerMoveItemFromSlotToContainer_Validate(
	UEISEquipmentSlot* FromEquipmentSlot, UEISItemContainer* ToContainer)
{
	return IsValid(FromEquipmentSlot) && IsValid(ToContainer);
}

void UEISInventoryManagerComponent::ServerMoveItemFromSlotToSlot_Implementation(
	UEISEquipmentSlot* FromEquipmentSlot, UEISEquipmentSlot* AtEquipmentSlot)
{
	InternalMoveItemFromSlotToSlot(FromEquipmentSlot, AtEquipmentSlot);
}

bool UEISInventoryManagerComponent::ServerMoveItemFromSlotToSlot_Validate(
	UEISEquipmentSlot* FromEquipmentSlot, UEISEquipmentSlot* AtEquipmentSlot)
{
	return IsValid(FromEquipmentSlot) && IsValid(AtEquipmentSlot);
}
