// Fill out your copyright notice in the Description page of Project Settings.

#include "EISItemInstance.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

int FEISItemAttributeModifierHandle::LastHandle {-1};

UEISItemAttributeContainer::UEISItemAttributeContainer(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UEISItemAttributeContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Attributes);
	DOREPLIFETIME(ThisClass, Modifiers);
}

void UEISItemAttributeContainer::Initialize()
{
	UEISItemInstance* ItemInstance = GetItemInstance();
	check(ItemInstance);

	const UEISItemDefinition* ItemDef = GetItemInstance()->GetDefinition();
	check(ItemDef);

	for (auto AttributeSet : ItemDef->AttributeSets)
	{
		for (auto Attribute : AttributeSet->Attributes)
		{
			const FGameplayTag& Tag = Attribute.Key;
			const FEISItemAttributeEntry& Value = Attribute.Value;
			Attributes.AddUnique(FEISItemAttributeData(Tag, Value));
		}
	}

	for (auto Attribute : ItemDef->AdditiveAttributes)
	{
		const FGameplayTag& Tag = Attribute.Key;
		const FEISItemAttributeEntry& Value = Attribute.Value;
		Attributes.AddUnique(FEISItemAttributeData(Tag, Value));
	}
}

FEISItemAttributeModifierHandle UEISItemAttributeContainer::ApplyAttributeModifier(
	const FEISItemAttributeModifier& ModifyData)
{
	for (const FEISItemAttributeData& Attribute : Attributes)
	{
		if (Attribute.AttributeTag == ModifyData.AttributeTag)
		{
			FEISItemAttributeModifierHandle ModHandle = FEISItemAttributeModifierHandle(ModifyData);
			if (ModHandle.IsValid())
			{
				ModHandle.Apply();
				Modifiers.Add(ModHandle);
				CalculateAttributes();
				return ModHandle;
			}
		}
	}
	return FEISItemAttributeModifierHandle();
}

bool UEISItemAttributeContainer::RemoveAttributeModifier(const FEISItemAttributeModifierHandle& ModHandle)
{
	for (uint8 i = 0; i < Modifiers.Num(); i++)
	{
		if (Modifiers[i].GetHandle() == ModHandle.GetHandle())
		{
			Modifiers.RemoveAt(i);
			CalculateAttributes();
			return true;
		}
	}
	return false;
}

bool UEISItemAttributeContainer::ToggleAttributeModifier(const FEISItemAttributeModifierHandle& ModHandle, bool bValue)
{
	for (uint8 i = 0; i < Modifiers.Num(); i++)
	{
		if (Modifiers[i].GetHandle() == ModHandle.GetHandle())
		{
			Modifiers[i].ToggleApply(bValue);
			CalculateAttributes();
			return true;
		}
	}
	return false;
}

void UEISItemAttributeContainer::CalculateAttributes()
{
	for (FEISItemAttributeData& Attribute : Attributes)
	{
		Attribute.Value = Attribute.GetDefaultValue();
		
		for (const FEISItemAttributeModifierHandle& ModHandle : Modifiers)
		{
			if (!ModHandle.IsValid() || !ModHandle.IsApplied() || Attribute.AttributeTag != ModHandle.GetTag())
			{
				continue;
			}

			switch (ModHandle.GetModType())
			{
			case EEISItemAttributeModifierType::Add:
				{
					float CalcValue = Attribute.Value + ModHandle.GetValue();
					Attribute.Value = FMath::Clamp(CalcValue, Attribute.GetMinValue(), Attribute.GetMaxValue());
					break;
				}
			case EEISItemAttributeModifierType::Subtract:
				{
					float CalcValue = Attribute.Value - ModHandle.GetValue();
					Attribute.Value = FMath::Clamp(CalcValue, Attribute.GetMinValue(), Attribute.GetMaxValue());
					break;
				}
			case EEISItemAttributeModifierType::Multiply:
				{
					float CalcValue = Attribute.Value * ModHandle.GetValue();
					Attribute.Value = FMath::Clamp(CalcValue, Attribute.GetMinValue(), Attribute.GetMaxValue());
					break;
				}
			case EEISItemAttributeModifierType::Divide:
				{
					float CalcValue = Attribute.Value / ModHandle.GetValue();
					Attribute.Value = FMath::Clamp(CalcValue, Attribute.GetMinValue(), Attribute.GetMaxValue());
					break;
				}
			default: break;
			}
		}
	}
}

FEISItemAttributeData UEISItemAttributeContainer::GetAttributeData(FGameplayTag AttributeTag) const
{
	for (const FEISItemAttributeData& Attribute : Attributes)
	{
		if (Attribute.AttributeTag == AttributeTag)
		{
			return Attribute;
		}
	}
	return FEISItemAttributeData();
}

UEISItemInstance* UEISItemAttributeContainer::GetItemInstance() const
{
	return GetTypedOuter<UEISItemInstance>();
}

UEISItemInstance* UEISItemInstanceComponent::GetOwner() const
{
	return GetTypedOuter<UEISItemInstance>();
}

UEISItemInstance::UEISItemInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	AttributeContainer = CreateDefaultSubobject<UEISItemAttributeContainer>("Attribute Container");
}

void UEISItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, AttributeContainer);
	DOREPLIFETIME(ThisClass, ItemInstanceData);
}

bool UEISItemInstance::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bReplicateSomething = false;
	if (AttributeContainer)
	{
		bReplicateSomething |= Channel->ReplicateSubobject(AttributeContainer.Get(), *Bunch, *RepFlags);
	}
	
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
		Amount)) || (ItemDefinition->bStackable && !ItemDefinition->bHasStackMaximum);
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

FEISItemAttributeModifierHandle UEISItemInstance::AddAttributeModifier(const FEISItemAttributeModifier& ModifyData)
{
	if (AttributeContainer)
	{
		return AttributeContainer->ApplyAttributeModifier(ModifyData);
	}
	return FEISItemAttributeModifierHandle();
}

bool UEISItemInstance::RemoveAttributeModifier(const FEISItemAttributeModifierHandle& ModifyHandle)
{
	if (AttributeContainer)
	{
		return AttributeContainer->RemoveAttributeModifier(ModifyHandle);
	}
	return false;
}

bool UEISItemInstance::ToggleAttributeModifier(const FEISItemAttributeModifierHandle& ModifyHandle, bool bValue)
{
	if (AttributeContainer)
	{
		return AttributeContainer->ToggleAttributeModifier(ModifyHandle, bValue);
	}
	return false;
}

FEISItemAttributeData UEISItemInstance::GetAttributeData(FGameplayTag AttributeTag) const
{
	if (AttributeContainer)
	{
		return AttributeContainer->GetAttributeData(AttributeTag);
	}
	return FEISItemAttributeData();
}

void UEISItemInstance::Initialize(int InItemId, const UEISItemInstance* SourceItem)
{
	ItemInstanceData.ItemId = InItemId;

	if (AttributeContainer)
	{
		AttributeContainer->Initialize();
	}
	
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
