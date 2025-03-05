// Fill out your copyright notice in the Description page of Project Settings.

#include "EISInventoryComponent.h"
#include "EISInventoryFunctionLibrary.h"
#include "EISItemContainer.h"

UEISInventoryComponent::UEISInventoryComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEISInventoryComponent::FindAvailablePlace(UEISItemInstance* Item)
{
	if (HasAuthority())
	{
		UEISInventoryFunctionLibrary::Container_FindAvailablePlace(ItemContainer, Item);
	}
}

void UEISInventoryComponent::AddItem(UEISItemInstance* Item)
{
	if (HasAuthority())
	{
		UEISInventoryFunctionLibrary::Container_AddItem(ItemContainer, Item);
	}
}

void UEISInventoryComponent::RemoveItem(UEISItemInstance* Item)
{
	if (HasAuthority())
	{
		UEISInventoryFunctionLibrary::Container_RemoveItem(ItemContainer, Item);
	}
}

void UEISInventoryComponent::StackItem(UEISItemInstance* SourceItem, UEISItemInstance* TargetItem)
{
	if (HasAuthority())
	{
		UEISInventoryFunctionLibrary::Container_StackItem(ItemContainer, SourceItem, TargetItem);
	}
}

void UEISInventoryComponent::SplitItem(UEISItemInstance* Item, int Amount)
{
	if (HasAuthority())
	{
		UEISInventoryFunctionLibrary::Container_SplitItem(ItemContainer, Item, Amount);
	}
}

void UEISInventoryComponent::SetItemContainer(UEISItemContainer* NewItemContainer)
{
	ItemContainer = NewItemContainer;
}

void UEISInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (ItemContainer)
		{
			ItemContainer->AddStartingData();
		}
	}
}
