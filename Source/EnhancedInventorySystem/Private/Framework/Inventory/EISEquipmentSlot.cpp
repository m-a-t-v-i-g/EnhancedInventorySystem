// Fill out your copyright notice in the Description page of Project Settings.

#include "EISEquipmentSlot.h"
#include "EISItemInstance.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

void UEISEquipmentSlot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, ItemInstance);
	DOREPLIFETIME(ThisClass, bAvailable);
}

bool UEISEquipmentSlot::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bReplicateSomething = false;
	if (ItemInstance)
	{
		bReplicateSomething |= Channel->ReplicateSubobject(ItemInstance.Get(), *Bunch, *RepFlags);
		bReplicateSomething |= ItemInstance->ReplicateSubobjects(Channel, Bunch, RepFlags);
	}
	
	return bReplicateSomething;
}

void UEISEquipmentSlot::SetupEquipmentSlot(FString InSlotName, FGameplayTagContainer InSlotTags)
{
	SlotName = InSlotName;
	CategoryTags = InSlotTags;
}

void UEISEquipmentSlot::AddStartingData()
{
}

bool UEISEquipmentSlot::CanEquipItem(const UEISItemInstance* Item) const
{
	check(Item);
	
	const UEISItemDefinition* Def = Item->GetDefinition();
	if (Def != nullptr)
	{
		return Def->Tags.HasAny(CategoryTags);
	}
	
	return false;
}

void UEISEquipmentSlot::CallLeaveItem(UEISItemInstance* Item)
{
	check(Item);
	
	EquipSlot(Item);
}

void UEISEquipmentSlot::CallRemoveItem(UEISItemInstance* Item)
{
	UnequipSlot();
}

void UEISEquipmentSlot::CallSubtractOrRemoveItem(UEISItemInstance* Item, int Amount)
{
	check(Item);

	if (GetItemInstance() == Item && Amount > 0)
	{
		int ItemAmount = Item->GetAmount();
		if (ItemAmount - Amount > 0)
		{
			Item->RemoveAmount(Amount);
		}
		else
		{
			UnequipSlot();
		}
	}
}

void UEISEquipmentSlot::EquipSlot(UEISItemInstance* InItemInstance)
{
	check(InItemInstance);

	ItemInstance = InItemInstance;
	ItemInstance->AddToEquipmentSlot(this);
	OnEquipmentSlotChangeDelegate.Broadcast(FEISEquipmentSlotChangeData(SlotName, ItemInstance.Get(), IsEquipped()));
	OnEquipmentSlotChange.Broadcast(FEISEquipmentSlotChangeData(SlotName, ItemInstance.Get(), IsEquipped()));
}

void UEISEquipmentSlot::UnequipSlot()
{
	check(ItemInstance);

	UEISItemInstance* PrevObject = ItemInstance;
	ItemInstance = nullptr;
	OnEquipmentSlotChangeDelegate.Broadcast(FEISEquipmentSlotChangeData(SlotName, PrevObject, IsEquipped()));
	OnEquipmentSlotChange.Broadcast(FEISEquipmentSlotChangeData(SlotName, PrevObject, IsEquipped()));
}

void UEISEquipmentSlot::SetAvailability(bool bInAvailability)
{
	bAvailable = bInAvailability;
	OnAvailabilityChange.Broadcast(bAvailable);
}

void UEISEquipmentSlot::OnRep_ItemInstance(UEISItemInstance* PrevItem)
{
	if (UEISItemInstance* ItemInst = IsEquipped() ? ItemInstance.Get() : PrevItem)
	{
		if (IsValid(ItemInstance.Get()))
		{
			ItemInstance->AddToEquipmentSlot(this);
		}

		OnEquipmentSlotChangeDelegate.Broadcast(FEISEquipmentSlotChangeData(SlotName, ItemInst, IsEquipped()));
		OnEquipmentSlotChange.Broadcast(FEISEquipmentSlotChangeData(SlotName, ItemInst, IsEquipped()));
	}
}

void UEISEquipmentSlot::OnRep_Availability()
{
	OnAvailabilityChange.Broadcast(bAvailable);
}