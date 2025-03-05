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
	ResetInventoryManager(GetPawn<APawn>());
	
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
		if (ToContainer->CanAddItem(Item))
		{
			UEISInventoryFunctionLibrary::Container_AddItem(ToContainer, Item);
			RemoveItemFromSource(FromSource, Item);
		}
		else
		{
			return;
		}
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
		if (Container->Contains(Item))
		{
			UEISInventoryFunctionLibrary::Container_RemoveItem(Container, Item);
		}
		else
		{
			return;
		}
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
		if (TargetItem->CanStackItem(SourceItem))
		{
			UEISInventoryFunctionLibrary::Container_StackItem(InContainer, SourceItem, TargetItem);
			RemoveItemFromSource(FromSource, SourceItem);
		}
		else
		{
			return;
		}
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
		else
		{
			return;
		}
	}

	ServerContainerSplitItem(Container, Item, Amount);
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
		if (AtEquipmentSlot->CanEquipItem(Item))
		{
			UEISInventoryFunctionLibrary::Slot_EquipItem(AtEquipmentSlot, Item);
			RemoveItemFromSource(FromSource, Item);
		}
		else
		{
			return;
		}
	}

	ServerSlotEquipItem(FromSource, AtEquipmentSlot, Item);
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
		if (EquipmentSlot->IsEquipped())
		{
			UEISInventoryFunctionLibrary::Slot_UnequipItem(EquipmentSlot);
		}
		else
		{
			return;
		}
	}

	ServerSlotUnequipItem(EquipmentSlot);
}

void UEISInventoryManagerComponent::RemoveItemFromSource(UObject* Source, UEISItemInstance* Item)
{
	UEISInventoryFunctionLibrary::RemoveItemFromSource(Source, Item);
}

void UEISInventoryManagerComponent::SubtractOrRemoveItemFromSource(UObject* Source, UEISItemInstance* Item, int Amount)
{
	UEISInventoryFunctionLibrary::SubtractOrRemoveItemFromSource(Source, Item, Amount);
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

void UEISInventoryManagerComponent::ServerContainerSplitItem_Implementation(UEISItemContainer* Container, UEISItemInstance* Item, int Amount)
{
	UEISInventoryFunctionLibrary::Container_SplitItem(Container, Item, Amount);
}

bool UEISInventoryManagerComponent::ServerContainerSplitItem_Validate(UEISItemContainer* Container, UEISItemInstance* Item, int Amount)
{
	return IsValid(Container) && IsValid(Item) && Amount > 0;
}

void UEISInventoryManagerComponent::ServerSlotEquipItem_Implementation(UObject* FromSource,
                                                                       UEISEquipmentSlot* AtEquipmentSlot,
                                                                       UEISItemInstance* Item)
{
	UEISInventoryFunctionLibrary::Slot_EquipItem(AtEquipmentSlot, Item);
	RemoveItemFromSource(FromSource, Item);
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
