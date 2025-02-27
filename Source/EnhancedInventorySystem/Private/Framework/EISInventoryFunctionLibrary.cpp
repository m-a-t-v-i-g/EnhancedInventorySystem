// Fill out your copyright notice in the Description page of Project Settings.

#include "EISInventoryFunctionLibrary.h"
#include "EISItemInstance.h"
#include "EISItemContainer.h"

int UEISInventoryFunctionLibrary::LastItemId {0};

UEISItemInstance* UEISInventoryFunctionLibrary::GenerateItem(UWorld* World, const UEISItemInstance* SourceItem)
{
	if (SourceItem)
	{
		if (UEISItemInstance* NewItem = NewObject<UEISItemInstance>(World, SourceItem->GetClass(),
		                                            FName(SourceItem->GetScriptName().ToString() + FString::Printf(
			                                            TEXT("_object%d"), LastItemId + 1))))
		{
			LastItemId++;
			NewItem->Initialize(LastItemId, SourceItem);
			return NewItem;
		}
	}
	return nullptr;
}

bool UEISInventoryFunctionLibrary::Container_FindAvailablePlace(UEISItemContainer* Container, UEISItemInstance* Item)
{
	if (!Container || !Item)
	{
		return false;
	}

	return Container->FindAvailablePlace(Item);
}

void UEISInventoryFunctionLibrary::Container_AddItem(UEISItemContainer* Container, UEISItemInstance* Item)
{
	if (!Container || !Item)
	{
		return;
	}

	Container->AddItem(Item);
}

void UEISInventoryFunctionLibrary::Container_RemoveItem(UEISItemContainer* Container, UEISItemInstance* Item)
{
	if (!Container || !Item)
	{
		return;
	}

	Container->RemoveItem(Item);
}

bool UEISInventoryFunctionLibrary::Container_StackItem(UEISItemContainer* Container, UEISItemInstance* SourceItem,
                                                       UEISItemInstance* TargetItem)
{
	if (!Container || !SourceItem || !TargetItem)
	{
		return false;
	}

	return Container->StackItem(SourceItem, TargetItem);
}

void UEISInventoryFunctionLibrary::Container_SplitItem(UEISItemContainer* Container, UEISItemInstance* Item, int Amount)
{
	if (!Container || !Item)
	{
		return;
	}

	Container->SplitItem(Item, Amount);
}

void UEISInventoryFunctionLibrary::Container_MoveItemToOtherContainer(UEISItemContainer* SourceContainer,
                                                                      UEISItemContainer* TargetContainer,
                                                                      UEISItemInstance* Item, bool bFullStack)
{
	if (!SourceContainer || !TargetContainer || !Item)
	{
		return;
	}
	
	if (!bFullStack && Item->GetAmount() > Item->GetStackAmount())
	{
		if (UEISItemInstance* StackableItem = TargetContainer->FindFirstStackForItem(Item))
		{
			StackableItem->AddAmount(Item->GetStackAmount());
		}
		else if (UEISItemInstance* NewItem = GenerateItem(TargetContainer->GetWorld(), Item))
		{
			NewItem->SetAmount(NewItem->GetStackAmount());
			Container_AddItem(TargetContainer, NewItem);
		}
		
		Item->RemoveAmount(Item->GetStackAmount());
	}
	else
	{
		if (Container_FindAvailablePlace(TargetContainer, Item))
		{
			Container_RemoveItem(SourceContainer, Item);
		}
	}
}
