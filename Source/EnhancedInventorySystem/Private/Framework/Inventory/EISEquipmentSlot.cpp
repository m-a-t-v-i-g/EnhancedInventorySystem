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
		bReplicateSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
		bReplicateSomething |= ItemInstance->ReplicateSubobjects(Channel, Bunch, RepFlags);
	}
	
	return bReplicateSomething;
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

void UEISEquipmentSlot::EquipSlot(UEISItemInstance* InItemInstance)
{
	check(InItemInstance);

	ItemInstance = InItemInstance;
	ItemInstance->OnAddToEquipmentSlot();
	OnEquipmentSlotChange.Broadcast(FEISEquipmentSlotChangeData(SlotName, ItemInstance.Get(), IsEquipped()));
}

void UEISEquipmentSlot::UnequipSlot()
{
	check(ItemInstance);

	UEISItemInstance* PrevObject = ItemInstance;
	ItemInstance = nullptr;
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
		if (ItemInstance)
		{
			ItemInstance->OnAddToEquipmentSlot();
		}
		
		OnEquipmentSlotChange.Broadcast(FEISEquipmentSlotChangeData(SlotName, ItemInst, IsEquipped()));
	}
}

void UEISEquipmentSlot::OnRep_Availability()
{
	OnAvailabilityChange.Broadcast(bAvailable);
}