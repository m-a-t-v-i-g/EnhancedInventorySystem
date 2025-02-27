// Fill out your copyright notice in the Description page of Project Settings.

#include "EISEquipmentComponent.h"
#include "EISEquipmentSlot.h"
#include "EISInventoryFunctionLibrary.h"

UEISEquipmentComponent::UEISEquipmentComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEISEquipmentComponent::EquipSlot(const FString& SlotName, UEISItemInstance* ItemInstance)
{
	if (HasAuthority())
	{
		UEISInventoryFunctionLibrary::Slot_EquipItem(FindEquipmentSlotByName(SlotName), ItemInstance);
	}
}

void UEISEquipmentComponent::UnequipSlot(const FString& SlotName)
{
	if (HasAuthority())
	{
		UEISInventoryFunctionLibrary::Slot_UnequipItem(FindEquipmentSlotByName(SlotName));
	}
}

UEISEquipmentSlot* UEISEquipmentComponent::FindEquipmentSlotByName(const FString& SlotName) const
{
	if (!EquipmentSlots.IsEmpty())
	{
		auto Predicate = EquipmentSlots.FindByPredicate([&, SlotName](const UEISEquipmentSlot* Slot)
		{
			return Slot->GetSlotName() == SlotName;
		});

		if (Predicate)
		{
			return *Predicate;
		}
	}
	return nullptr;
}

void UEISEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
}
