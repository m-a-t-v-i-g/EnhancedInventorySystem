// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "EISItemInstance.generated.h"

class UEISItemInstanceComponent;
class UEISItemInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemInstanceSignature, UEISItemInstance*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FItemIntValueChangeSignature, int, NewAmount, int, PrevAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FItemFloatValueChangeSignature, float, NewAmount, float, PrevAmount);

USTRUCT(DisplayName = "Item Instance Data", BlueprintType, Blueprintable)
struct FEISItemInstanceData
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(VisibleInstanceOnly, Category = "Item")
	int ItemId = 0;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Item", meta = (ClampMin = "1"))
	int Amount = 1;
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

UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, Within = "EISItemDefinition")
class ENHANCEDINVENTORYSYSTEM_API UEISItemInstanceComponent : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = "Item Component")
	UEISItemInstance* GetOwner() const;
	
	template <class T>
	const T* GetOwner() const
	{
		return Cast<const T>(GetOwner());
	}
};

UCLASS(DisplayName = "Item Instance", Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class ENHANCEDINVENTORYSYSTEM_API UEISItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UEISItemInstance(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable)
	FItemInstanceSignature OnItemCreate;
	
	UPROPERTY(BlueprintAssignable)
	FItemIntValueChangeSignature OnAmountChange;
	
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
	TArray<UEISItemInstanceComponent*> GetComponents() const;
	
#pragma endregion Components

#pragma region Item Interface

	void Initialize(int InItemId, const UEISItemInstance* SourceItem);

	virtual void OnInitialize(const UEISItemInstance* SourceItem);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnInitialize")
	void K2_OnInitialize(const UEISItemInstance* SourceItem);
	
	void AddToContainer();
	virtual void OnAddToContainer();
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnAddToContainer")
	void K2_OnAddToContainer();
	
	UFUNCTION(BlueprintPure, Category = "Item")
	virtual bool CanStackItem(const UEISItemInstance* OtherItem) const;

	UFUNCTION(BlueprintPure, Category = "Item")
	virtual bool IsMatchItem(const UEISItemInstance* OtherItem) const;

#pragma endregion Item Interface

protected:
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Item")
	FEISItemInstanceData ItemInstanceData;
	
private:
	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<UEISItemDefinition> ItemDefinition;
};
