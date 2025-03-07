// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "EISItemInstance.generated.h"

class UEISItemInstanceComponent;
class UEISItemInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemInstanceSignature, UEISItemInstance*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FItemIntValueChangeSignature, UEISItemInstance*, Item, int, NewAmount,
                                               int, PrevAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FItemFloatValueChangeSignature, UEISItemInstance*, Item, float,
                                               NewAmount, float, PrevAmount);

UENUM()
enum class EEISItemAttributeModifierType : uint8
{
	Add,
	Subtract,
	Multiply,
	Divide
};

USTRUCT(DisplayName = "Item Instance Data", BlueprintType, Blueprintable)
struct FEISItemInstanceData
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(VisibleInstanceOnly, Category = "Item")
	int ItemId = 0;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Item", meta = (ClampMin = "1"))
	int Amount = 1;
};

USTRUCT(Blueprintable)
struct FEISItemAttributeEntry
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Attribute")
	float DefaultValue = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Attribute")
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Attribute")
	float MaxValue = 0.0f;
	
	FEISItemAttributeEntry()
	{
	}

	FEISItemAttributeEntry(float DefVal, float MinVal, float MaxVal) : DefaultValue(DefVal), MinValue(MinVal),
	                                                                   MaxValue(MaxVal)
	{
	}
};

USTRUCT(DisplayName = "Item Attribute Data", BlueprintType, Blueprintable)
struct FEISItemAttributeData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	FGameplayTag AttributeTag;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	float Value = 0.0f;

	FEISItemAttributeData()
	{
	}

	FEISItemAttributeData(const FGameplayTag& AttrTag, const FEISItemAttributeEntry& AttrEntry) : AttributeTag(AttrTag),
		Value(AttrEntry.DefaultValue), ItemAttributeEntry(AttrEntry)
	{
	}

	float GetDefaultValue() const
	{
		return ItemAttributeEntry.DefaultValue;
	}

	float GetMinValue() const
	{
		return ItemAttributeEntry.MinValue;
	}
	
	float GetMaxValue() const
	{
		return ItemAttributeEntry.MaxValue;
	}
	
	bool IsValid() const
	{
		return AttributeTag.IsValid();
	}

	bool operator==(const FEISItemAttributeData& OtherAttr) const
	{
		return AttributeTag == OtherAttr.AttributeTag;
	}

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FEISItemAttributeEntry ItemAttributeEntry;
};

USTRUCT(DisplayName = "Item Attribute Modifier", BlueprintType, Blueprintable)
struct FEISItemAttributeModifier
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag AttributeTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEISItemAttributeModifierType ModType = EEISItemAttributeModifierType::Add;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.0f;

	void Apply()
	{
		bApplied = true;
	}

	void Unapply()
	{
		bApplied = false;
	}

	bool IsValid() const
	{
		return AttributeTag.IsValid();
	}

	bool Applied() const
	{
		return bApplied;
	}
	
private:
	UPROPERTY(VisibleInstanceOnly)
	bool bApplied = false;
};

USTRUCT(DisplayName = "Item Attribute Modifier Handle", BlueprintType, Blueprintable)
struct FEISItemAttributeModifierHandle
{
	GENERATED_USTRUCT_BODY()

	FEISItemAttributeModifierHandle()
	{
		Handle = -1;
	}

	FEISItemAttributeModifierHandle(const FEISItemAttributeModifier& NewAttributeModifier) : ModifierData(
		NewAttributeModifier)
	{
		LastHandle++;
		Handle = LastHandle;
	}

	void Apply()
	{
		ModifierData.Apply();
	}

	void Unapply()
	{
		ModifierData.Unapply();
	}
				
	void ToggleApply(bool bValue)
	{
		bValue ? ModifierData.Apply() : ModifierData.Unapply();
	}

	int GetHandle() const
	{
		return Handle;
	}
	
	const FGameplayTag& GetTag() const
	{
		return ModifierData.AttributeTag;
	}
	
	EEISItemAttributeModifierType GetModType() const
	{
		return ModifierData.ModType;
	}
	
	float GetValue() const
	{
		return ModifierData.Value;
	}
	
	bool IsValid() const
	{
		return Handle >= 0;
	}

	bool IsApplied() const
	{
		return ModifierData.Applied();
	}

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int Handle = -1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FEISItemAttributeModifier ModifierData;

	static int LastHandle;
};

UCLASS(DisplayName = "Item Attribute Asset")
class ENHANCEDINVENTORYSYSTEM_API UEISItemAttributeAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Attribute Asset")
	TMap<FGameplayTag, FEISItemAttributeEntry> Attributes;
};

UCLASS()
class ENHANCEDINVENTORYSYSTEM_API UEISItemAttributeContainer : public UObject
{
	GENERATED_BODY()

public:
	UEISItemAttributeContainer(const FObjectInitializer& ObjectInitializer);

#pragma region Replication
	
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma endregion Replication

	void Initialize();

	FEISItemAttributeModifierHandle ApplyAttributeModifier(const FEISItemAttributeModifier& ModifyData);
	bool RemoveAttributeModifier(const FEISItemAttributeModifierHandle& ModHandle);
	bool ToggleAttributeModifier(const FEISItemAttributeModifierHandle& ModHandle, bool bValue);
	void CalculateAttributes();

	TArray<FEISItemAttributeData> GetAttributes() const;
	FEISItemAttributeData GetAttributeData(FGameplayTag AttributeTag) const;
	
	UFUNCTION(BlueprintPure, Category = "Attribute Container")
	UEISItemInstance* GetItemInstance() const;
	
	template <class T>
	T* GetItemInstance() const
	{
		return Cast<T>(GetItemInstance());
	}

private:
	friend UEISItemInstance;
	
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Attribute Container")
	TArray<FEISItemAttributeData> Attributes;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Attribute Container")
	TArray<FEISItemAttributeModifierHandle> Modifiers;
};

UCLASS(DisplayName = "Item Definition")
class ENHANCEDINVENTORYSYSTEM_API UEISItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Class")
	FName ScriptName;

	UPROPERTY(EditAnywhere, Category = "Class")
	FGameplayTagContainer Tags;
	
	UPROPERTY(EditAnywhere, Instanced, Category = "Components")
	TArray<UEISItemInstanceComponent*> Components;
	
	UPROPERTY(EditAnywhere, Category = "Attributes")
	TArray<TObjectPtr<const UEISItemAttributeAsset>> AttributeSets;
	
	UPROPERTY(EditAnywhere, Category = "Attributes")
	TMap<FGameplayTag, FEISItemAttributeEntry> AdditiveAttributes;
	
	UPROPERTY(EditAnywhere, Category = "Properties|Stacking")
	bool bStackable = false;
	
	UPROPERTY(EditAnywhere, Category = "Properties|Stacking", meta = (EditCondition = "bStackable", ClampMin = "1"))
	int StackAmount = 1;
	
	UPROPERTY(EditAnywhere, Category = "Properties|Stacking", meta = (EditCondition = "bStackable"))
	bool bHasStackMaximum = false;
	
	UPROPERTY(EditAnywhere, Category = "Properties|Stacking",
		meta = (EditCondition = "bStackable && bHasStackMaximum", ClampMin = "1"))
	int StackMaximum = 1;
};

UCLASS(DisplayName = "Item Instance Component", Abstract, BlueprintType, Blueprintable, EditInlineNew,
	DefaultToInstanced, Within = "EISItemDefinition")
class ENHANCEDINVENTORYSYSTEM_API UEISItemInstanceComponent : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = "Item Component")
	UEISItemInstance* GetOwner() const;
	
	template <class T>
	T* GetOwner() const
	{
		return Cast<T>(GetOwner());
	}
};

UCLASS(DisplayName = "Item Instance", Abstract, BlueprintType, Blueprintable)
class ENHANCEDINVENTORYSYSTEM_API UEISItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UEISItemInstance(const FObjectInitializer& ObjectInitializer);

#pragma region Delegates
	
	TMulticastDelegate<void(UEISItemInstance*)> OnItemCreateDelegate;
	
	UPROPERTY(BlueprintAssignable)
	FItemInstanceSignature OnItemCreate;

	TMulticastDelegate<void(int, int)> OnAmountChangeDelegate;
	
	UPROPERTY(BlueprintAssignable)
	FItemIntValueChangeSignature OnAmountChange;
	
#pragma endregion Delegates

#pragma region Replication
	
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags);

#pragma endregion Replication

#pragma region Definition
	
	UFUNCTION(BlueprintPure, Category = "Item|Definition")
	const UEISItemDefinition* GetDefinition() const;
	
	template <class T>
	const T* GetDefinition() const
	{
		return Cast<const T>(GetDefinition());
	}
	
	UFUNCTION(BlueprintPure, Category = "Item|Definition")
	FORCEINLINE FName GetScriptName() const;
	
	UFUNCTION(BlueprintPure, Category = "Item|Definition")
	const FGameplayTagContainer& GetTags() const;
	
	UFUNCTION(BlueprintPure, Category = "Item|Definition")
	FORCEINLINE bool IsStackable() const;
	
	UFUNCTION(BlueprintPure, Category = "Item|Definition")
	FORCEINLINE int GetStackAmount() const;

#pragma endregion Definition

#pragma region Amount
	
	UFUNCTION(BlueprintCallable, Category = "Item|Amount")
	void SetAmount(int InAmount);
	
	UFUNCTION(BlueprintCallable, Category = "Item|Amount")
	int AddAmount(int InAmount);
	
	UFUNCTION(BlueprintCallable, Category = "Item|Amount")
	int RemoveAmount(int InAmount);
	
	virtual void OnUpdateAmount(int NewAmount, int PrevAmount);
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnUpdateAmount")
	void K2_OnUpdateAmount(int NewAmount, int PrevAmount);
	
	UFUNCTION(BlueprintPure, Category = "Item|Amount")
	int GetAmount() const { return ItemInstanceData.Amount; }
	
#pragma endregion Amount

#pragma region Components
	
	UFUNCTION(BlueprintPure, Category = "Item|Components")
	UEISItemInstanceComponent* GetComponentByClass(TSubclassOf<UEISItemInstanceComponent> ItemComponentClass) const;
	
	template <class T>
	T* GetComponentByClass(UClass* ItemComponentClass) const
	{
		return Cast<T>(GetComponentByClass(ItemComponentClass));
	}
	
	UFUNCTION(BlueprintPure, Category = "Item|Components")
	TArray<UEISItemInstanceComponent*> GetComponents() const;
	
#pragma endregion Components

#pragma region Attributes

	UFUNCTION(BlueprintCallable, Category = "Item|Attributes")
	FEISItemAttributeModifierHandle AddAttributeModifier(const FEISItemAttributeModifier& ModifyData);

	UFUNCTION(BlueprintCallable, Category = "Item|Attributes")
	bool RemoveAttributeModifier(const FEISItemAttributeModifierHandle& ModifyHandle);

	UFUNCTION(BlueprintCallable, Category = "Item|Attributes")
	bool ToggleAttributeModifier(const FEISItemAttributeModifierHandle& ModifyHandle, bool bValue);

	UFUNCTION(BlueprintPure, Category = "Item|Attributes")
	TArray<FEISItemAttributeData> GetAttributes() const;
	
	UFUNCTION(BlueprintPure, Category = "Item|Attributes")
	FEISItemAttributeData GetAttributeData(FGameplayTag AttributeTag) const;
	
#pragma endregion Attributes

#pragma region Item Interface

	void Initialize(int InItemId, const UEISItemInstance* SourceItem);

	virtual void OnInitialize(const UEISItemInstance* SourceItem);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnInitialize")
	void K2_OnInitialize(const UEISItemInstance* SourceItem);

	void AddToContainer(UObject* Owner);
	virtual void OnAddToContainer(UObject* Owner);
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnAddToContainer")
	void K2_OnAddToContainer(UObject* Owner);
	
	void AddToEquipmentSlot(UObject* Owner);
	virtual void OnAddToEquipmentSlot(UObject* Owner);
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnAddToEquipmentSlot")
	void K2_AddToEquipmentSlot(UObject* Owner);
	
	UFUNCTION(BlueprintPure, Category = "Item")
	virtual bool CanStackItem(const UEISItemInstance* OtherItem) const;

	UFUNCTION(BlueprintPure, Category = "Item")
	virtual bool IsMatchItem(const UEISItemInstance* OtherItem) const;

	UFUNCTION(BlueprintPure, Category = "Item")
	int GetItemId() const { return ItemInstanceData.ItemId; }
	
	UFUNCTION(BlueprintPure, Category = "Item")
	UObject* GetOwner() const { return OwnerPrivate; }
	
#pragma endregion Item Interface

protected:
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Item")
	FEISItemInstanceData ItemInstanceData;

private:
	void SetOwner(UObject* Owner);
	
	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<UEISItemDefinition> ItemDefinition;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Item")
	TObjectPtr<UEISItemAttributeContainer> AttributeContainer;

	UPROPERTY()
	TObjectPtr<UObject> OwnerPrivate;
};
