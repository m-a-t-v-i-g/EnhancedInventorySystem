// Fill out your copyright notice in the Description page of Project Settings.

#include "EISInventoryFunctionLibrary.h"
#include "EISItem.h"

int UEISInventoryFunctionLibrary::LastItemId {0};

UEISItem* UEISInventoryFunctionLibrary::GenerateItem(UWorld* World, const UEISItem* Item)
{
	if (Item)
	{
		if (UEISItem* NewItem = NewObject<UEISItem>(World, Item->GetClass(),
		                                            FName(Item->GetScriptName().ToString() + FString::Printf(
			                                            TEXT("_object%d"), LastItemId + 1))))
		{
			LastItemId++;
			return NewItem;
		}
	}
	return nullptr;
}
