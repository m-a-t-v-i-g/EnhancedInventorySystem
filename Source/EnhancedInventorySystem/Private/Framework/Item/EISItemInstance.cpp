// Fill out your copyright notice in the Description page of Project Settings.

#include "EISItemInstance.h"
#include "Net/UnrealNetwork.h"

UEISItemInstance* UEISItemInstanceComponent::GetOwner() const
{
	return GetTypedOuter<UEISItemInstance>();
}

UEISItemInstance::UEISItemInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UEISItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemInstanceData);
}

bool UEISItemInstance::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bReplicateSomething = false;
	return bReplicateSomething;
}

const UEISItemDefinition* UEISItemInstance::GetDefinition() const
{
	return ItemDefinition.Get();
}

FName UEISItemInstance::GetScriptName() const
{
	return ItemDefinition->ScriptName;
}

const FGameplayTagContainer& UEISItemInstance::GetTags() const
{
	return ItemDefinition->Tags;
}

bool UEISItemInstance::IsStackable() const
{
	return (ItemDefinition->bStackable && (ItemDefinition->bHasStackMaximum && ItemDefinition->StackMaximum > ItemInstanceData.
		Amount)) || ItemDefinition->bStackable;
}

int UEISItemInstance::GetStackAmount() const
{
	return ItemDefinition->StackAmount;
}

void UEISItemInstance::SetAmount(int InAmount)
{
	int PrevAmount = ItemInstanceData.Amount;
	int NewAmount = ItemInstanceData.Amount = InAmount;
	
	OnAmountChangeDelegate.Broadcast(NewAmount, PrevAmount);
	OnAmountChange.Broadcast(this, NewAmount, PrevAmount);
	
	OnUpdateAmount(NewAmount, PrevAmount);
	K2_OnUpdateAmount(NewAmount, PrevAmount);
}

int UEISItemInstance::AddAmount(int InAmount)
{
	int PrevAmount = ItemInstanceData.Amount;
	int NewAmount = ItemInstanceData.Amount += InAmount;
	
	OnAmountChangeDelegate.Broadcast(NewAmount, PrevAmount);
	OnAmountChange.Broadcast(this, NewAmount, PrevAmount);
	
	OnUpdateAmount(NewAmount, PrevAmount);
	K2_OnUpdateAmount(NewAmount, PrevAmount);
	return NewAmount;
}

int UEISItemInstance::RemoveAmount(int InAmount)
{
	int PrevAmount = ItemInstanceData.Amount;
	int NewAmount = ItemInstanceData.Amount -= InAmount;
	
	OnAmountChangeDelegate.Broadcast(NewAmount, PrevAmount);
	OnAmountChange.Broadcast(this, NewAmount, PrevAmount);
	
	OnUpdateAmount(NewAmount, PrevAmount);
	K2_OnUpdateAmount(NewAmount, PrevAmount);
	return NewAmount;
}

void UEISItemInstance::OnUpdateAmount(int NewAmount, int PrevAmount)
{
}

UEISItemInstanceComponent* UEISItemInstance::GetComponentByClass(TSubclassOf<UEISItemInstanceComponent> ItemComponentClass) const
{
	TArray<UEISItemInstanceComponent*> Components = GetDefinition()->Components;
	for (UEISItemInstanceComponent* Component : Components)
	{
		check(Component);
		
		if (Component->IsA(ItemComponentClass))
		{
			return Component;
		}
	}
	return nullptr;
}

TArray<UEISItemInstanceComponent*> UEISItemInstance::GetComponents() const
{
	return GetDefinition()->Components;
}

void UEISItemInstance::Initialize(int InItemId, const UEISItemInstance* SourceItem)
{
	ItemInstanceData.ItemId = InItemId;
	
	OnInitialize(SourceItem);
	K2_OnInitialize(SourceItem);
	
	OnItemCreateDelegate.Broadcast(this);
	OnItemCreate.Broadcast(this);
}

void UEISItemInstance::OnInitialize(const UEISItemInstance* SourceItem)
{
}

void UEISItemInstance::AddToContainer(UObject* Owner)
{
	SetOwner(Owner);
	
	OnAddToContainer(Owner);
	K2_OnAddToContainer(Owner);
}

void UEISItemInstance::OnAddToContainer(UObject* Owner)
{
	
}

void UEISItemInstance::AddToEquipmentSlot(UObject* Owner)
{
	SetOwner(Owner);
	
	OnAddToEquipmentSlot(Owner);
	K2_AddToEquipmentSlot(Owner);
}

void UEISItemInstance::OnAddToEquipmentSlot(UObject* Owner)
{
	
}

bool UEISItemInstance::CanStackItem(const UEISItemInstance* OtherItem) const
{
	check(OtherItem);
	return OtherItem != this && IsStackable() && IsMatchItem(OtherItem);
}

bool UEISItemInstance::IsMatchItem(const UEISItemInstance* OtherItem) const
{
	check(OtherItem);
	return GetDefinition() == OtherItem->GetDefinition();
}

void UEISItemInstance::SetOwner(UObject* Owner)
{
	OwnerPrivate = Owner;
}
