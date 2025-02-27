// Fill out your copyright notice in the Description page of Project Settings.

#include "EISItemContainer.h"
#include "EISInventoryFunctionLibrary.h"
#include "EISItemInstance.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

void UEISItemContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, Items);
}

bool UEISItemContainer::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bReplicateSomething = false;
	for (UEISItemInstance* Item : GetItems())
	{
		bReplicateSomething |= Channel->ReplicateSubobject(Item, *Bunch, *RepFlags);
		bReplicateSomething |= Item->ReplicateSubobjects(Channel, Bunch, RepFlags);
	}

	return bReplicateSomething;
}

void UEISItemContainer::AddStartingData()
{
	for (UEISItemInstance* Item : StartingData)
	{
		if (!IsValid(Item))
		{
			continue;
		}

		const UEISItemInstance* ItemCDO = Item->GetClass()->GetDefaultObject<UEISItemInstance>();
		check(ItemCDO);

		if (Item->GetDefinition() == ItemCDO->GetDefinition())
		{
			if (CanAddItem(Item))
			{
				AddItem(Item);
			}
		}
		else
		{
			// Log
		}
	}
	
	StartingData.Empty();
}

bool UEISItemContainer::CanAddItem(const UEISItemInstance* Item) const
{
	check(Item);
	
	const UEISItemDefinition* Def = Item->GetDefinition();
	if (Def != nullptr)
	{
		return Def->Tags.HasAny(CategoryTags);
	}
	
	return false;
}

bool UEISItemContainer::Contains(const UEISItemInstance* Item) const
{
	return Items.Contains(Item);
}

UEISItemInstance* UEISItemContainer::FindFirstStackForItem(const UEISItemInstance* ForItem) const
{
	for (int i = 0; i < Items.Num(); i++)
	{
		auto FoundItem = Items[i];
		if (!FoundItem)
		{
			continue;
		}
		
		if (FoundItem->CanStackItem(ForItem))
		{
			return FoundItem;
		}
	}
	return nullptr;
}

UEISItemInstance* UEISItemContainer::FindItemByDefinition(const UEISItemDefinition* Definition) const
{
	for (UEISItemInstance* Item : Items)
	{
		const UEISItemDefinition* Def = Item->GetDefinition();
		if (Def == Definition)
		{
			return Item;
		}
	}
	return nullptr;
}

UEISItemInstance* UEISItemContainer::FindItemByName(const FName& ScriptName) const
{
	for (UEISItemInstance* Item : Items)
	{
		const UEISItemDefinition* Def = Item->GetDefinition();
		if (Def->ScriptName == ScriptName)
		{
			return Item;
		}
	}
	return nullptr;
}

bool UEISItemContainer::FindAvailablePlace(UEISItemInstance* Item)
{
	if (Item)
	{
		if (UEISItemInstance* StackableItem = FindFirstStackForItem(Item))
		{
			StackItem(Item, StackableItem);
			return true;
		}
		
		if (CanAddItem(Item))
		{
			AddItem(Item);
			return true;
		}
	}
	return false;
}

void UEISItemContainer::AddItem(UEISItemInstance* Item)
{
	if (Item && CanAddItem(Item))
	{
		Items.Add(Item);
		Item->OnAddToContainer();
		OnContainerChange.Broadcast(FEISItemContainerChangeData(TArray{Item}, {}));
	}
}

void UEISItemContainer::RemoveItem(UEISItemInstance* Item)
{
	if (Item && Items.Contains(Item))
	{
		Items.Remove(Item);
		OnContainerChange.Broadcast(FEISItemContainerChangeData({}, TArray{Item}));
	}
}

bool UEISItemContainer::StackItem(UEISItemInstance* SourceItem, UEISItemInstance* TargetItem)
{
	if (SourceItem && TargetItem)
	{
		if (TargetItem->CanStackItem(SourceItem))
		{
			TargetItem->AddAmount(SourceItem->GetAmount());
			RemoveItem(SourceItem);
			return true;
		}
	}
	return false;
}

bool UEISItemContainer::SplitItem(UEISItemInstance* Item, int Amount)
{
	if (Item && CanAddItem(Item))
	{
		if (Item->GetAmount() > 1 && Item->GetAmount() > Amount)
		{
			UEISItemInstance* SplitItem = UEISInventoryFunctionLibrary::GenerateItem(GetWorld(), Item);
			if (SplitItem != nullptr)
			{
				Item->RemoveAmount(Amount);
				SplitItem->SetAmount(Amount);
				AddItem(SplitItem);
			}
		}
	}
	return false;
}

void UEISItemContainer::OnRep_Items(TArray<UEISItemInstance*> PrevContainer)
{
	TArray<UEISItemInstance*> AddedItems;
	TArray<UEISItemInstance*> RemovedItems;
	
	for (UEISItemInstance* Item : Items)
	{
		if (PrevContainer.Contains(Item))
		{
			continue;
		}

		AddedItems.AddUnique(Item);
	}
	
	for (UEISItemInstance* Item : PrevContainer)
	{
		if (Items.Contains(Item))
		{
			continue;
		}
		
		RemovedItems.Remove(Item);
	}

	OnContainerChange.Broadcast(FEISItemContainerChangeData(AddedItems, RemovedItems));
}
