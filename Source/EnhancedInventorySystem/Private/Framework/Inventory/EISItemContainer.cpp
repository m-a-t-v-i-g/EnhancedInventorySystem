// Fill out your copyright notice in the Description page of Project Settings.

#include "EISItemContainer.h"
#include "EISInventoryFunctionLibrary.h"
#include "EISItem.h"

void UEISItemContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
}

bool UEISItemContainer::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	return true;
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
		if (UEISItem* StackableItem = FindStackForItem(Item))
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
	}
}

void UEISItemContainer::RemoveItem(UEISItem* Item)
{
	if (Item && Items.Contains(Item))
	{
		Items.Remove(Item);
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

bool UEISItemContainer::CanAddItem(const UEISItem* Item) const
{
	check(Item);
	
	const UEISItemDefinition* Def = Item->GetDefinition();
	if (Def != nullptr)
	{
		return Def->Tag.MatchesAny(CategoryTags);
	}
	
	return false;
}

bool UEISItemContainer::Contains(const UEISItem* Item) const
{
	return Items.Contains(Item);
}

UEISItem* UEISItemContainer::FindStackForItem(const UEISItem* ForItem) const
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

void UEISItemContainer::OnRep_Items(TArray<UEISItem*> PrevContainer)
{
}
