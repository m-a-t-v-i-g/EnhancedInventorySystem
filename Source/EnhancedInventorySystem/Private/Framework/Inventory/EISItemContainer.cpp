// Fill out your copyright notice in the Description page of Project Settings.

#include "EISItemContainer.h"
#include "EISInventoryFunctionLibrary.h"
#include "EISItem.h"
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
	for (UEISItem* Item : GetItems())
	{
		bReplicateSomething |= Channel->ReplicateSubobject(Item, *Bunch, *RepFlags);
		bReplicateSomething |= Item->ReplicateSubobjects(Channel, Bunch, RepFlags);
	}

	return bReplicateSomething;
}

bool UEISItemContainer::CanAddItem(const UEISItem* Item) const
{
	check(Item);
	
	const UEISItemDefinition* Def = Item->GetDefinition();
	if (Def != nullptr)
	{
		return Def->Tags.HasAny(CategoryTags);
	}
	
	return false;
}

bool UEISItemContainer::Contains(const UEISItem* Item) const
{
	return Items.Contains(Item);
}

UEISItem* UEISItemContainer::FindFirstStackForItem(const UEISItem* ForItem) const
{
	for (int i = 0; i < Items.Num(); i++)
	{
		auto FoundItem = Items[i];
		if (!FoundItem)
		{
			continue;
		}
		
		if (FoundItem->CanStackWith(ForItem))
		{
			return FoundItem;
		}
	}
	return nullptr;
}

UEISItem* UEISItemContainer::FindItemByDefinition(const UEISItemDefinition* Definition) const
{
	for (UEISItem* Item : Items)
	{
		const UEISItemDefinition* Def = Item->GetDefinition();
		if (Def == Definition)
		{
			return Item;
		}
	}
	return nullptr;
}

UEISItem* UEISItemContainer::FindItemByName(const FName& ScriptName) const
{
	for (UEISItem* Item : Items)
	{
		const UEISItemDefinition* Def = Item->GetDefinition();
		if (Def->ScriptName == ScriptName)
		{
			return Item;
		}
	}
	return nullptr;
}

void UEISItemContainer::AddStartingData()
{
	for (UEISItem* Item : StartingData)
	{
		if (!IsValid(Item))
		{
			continue;
		}

		const UEISItem* ItemCDO = Item->GetClass()->GetDefaultObject<UEISItem>();
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

bool UEISItemContainer::FindAvailablePlace(UEISItem* Item)
{
	if (Item)
	{
		if (UEISItem* StackableItem = FindFirstStackForItem(Item))
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

void UEISItemContainer::AddItem(UEISItem* Item)
{
	if (Item && CanAddItem(Item))
	{
		Items.Add(Item);
		OnContainerChange.Broadcast(FEISItemContainerChangeData(TArray{Item}, {}));
	}
}

void UEISItemContainer::RemoveItem(UEISItem* Item)
{
	if (Item && Items.Contains(Item))
	{
		Items.Remove(Item);
		OnContainerChange.Broadcast(FEISItemContainerChangeData({}, TArray{Item}));
	}
}

bool UEISItemContainer::StackItem(UEISItem* SourceItem, UEISItem* TargetItem)
{
	if (SourceItem && TargetItem)
	{
		if (TargetItem->CanStackWith(SourceItem))
		{
			TargetItem->AddAmount(SourceItem->GetAmount());
			RemoveItem(SourceItem);
			return true;
		}
	}
	return false;
}

bool UEISItemContainer::SplitItem(UEISItem* Item, int Amount)
{
	if (Item && CanAddItem(Item))
	{
		if (Item->GetAmount() > 1 && Item->GetAmount() > Amount)
		{
			UEISItem* SplitItem = UEISInventoryFunctionLibrary::GenerateItem(GetWorld(), Item);
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

void UEISItemContainer::OnRep_Items(TArray<UEISItem*> PrevContainer)
{
	TArray<UEISItem*> AddedItems;
	TArray<UEISItem*> RemovedItems;
	
	for (UEISItem* Item : Items)
	{
		if (PrevContainer.Contains(Item))
		{
			continue;
		}

		AddedItems.AddUnique(Item);
	}
	
	for (UEISItem* Item : PrevContainer)
	{
		if (Items.Contains(Item))
		{
			continue;
		}
		
		RemovedItems.Remove(Item);
	}

	OnContainerChange.Broadcast(FEISItemContainerChangeData(AddedItems, RemovedItems));
}
