// Fill out your copyright notice in the Description page of Project Settings.

#include "EISItem.h"

UEISItem::UEISItem(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UEISItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
}

const UEISItemDefinition* UEISItem::GetDefinition() const
{
	return ItemDefinition.Get();
}

FName UEISItem::GetScriptName() const
{
	return ItemDefinition->ScriptName;
}

bool UEISItem::IsStackable() const
{
	return ItemDefinition->bStackable;
}

void UEISItem::SetAmount(int InAmount)
{
	int PrevAmount = Amount;
	int NewAmount = Amount = InAmount;
	OnAmountChange.Broadcast(NewAmount, PrevAmount);
}

int UEISItem::AddAmount(int InAmount)
{
	int PrevAmount = Amount;
	int NewAmount = Amount += InAmount;
	OnAmountChange.Broadcast(NewAmount, PrevAmount);
	return NewAmount;
}

int UEISItem::RemoveAmount(int InAmount)
{
	int PrevAmount = Amount;
	int NewAmount = Amount -= InAmount;
	OnAmountChange.Broadcast(NewAmount, PrevAmount);
	return NewAmount;
}

bool UEISItem::CanStackWith(const UEISItem* OtherItem) const
{
	return OtherItem != this && IsStackable() && IsCorrespondsTo(OtherItem);
}

bool UEISItem::IsCorrespondsTo(const UEISItem* OtherItem) const
{
	check(OtherItem);
	
	bool bResult = GetDefinition() == OtherItem->GetDefinition();
	if (bResult)
	{
		for (uint8 i = 0; i < Components.Num(); i++)
		{
			if (OtherItem->Components.IsValidIndex(i))
			{
				bResult &= Components[i] == OtherItem->Components[i];
				if (!bResult)
				{
					break;
				}
			}
		}
	}
	return bResult;
}
