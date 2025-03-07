// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EISItemInstance.h"
#include "EISItemRepositoryInterface.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "EISItemContainer.generated.h"

class UEISInventoryFunctionLibrary;
class UEISItemInstance;

USTRUCT(Blueprintable)
struct FEISItemContainerChangeData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<UEISItemInstance*> AddedItems;

	UPROPERTY(BlueprintReadOnly)
	TArray<UEISItemInstance*> RemovedItems;

	FEISItemContainerChangeData()
	{
	}
	
	FEISItemContainerChangeData(const TArray<UEISItemInstance*>& NewItems, const TArray<UEISItemInstance*>& OldItems)
	{
		AddedItems = NewItems;
		RemovedItems = OldItems;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContainerChangeSignature, const FEISItemContainerChangeData&,
                                            ContainerChangeData);

UCLASS(DisplayName = "Item Container", Abstract, EditInlineNew, DefaultToInstanced)
class ENHANCEDINVENTORYSYSTEM_API UEISItemContainer : public UObject, public IEISItemRepositoryInterface
{
	GENERATED_BODY()

	friend UEISInventoryFunctionLibrary;
	
public:
	TMulticastDelegate<void(const FEISItemContainerChangeData&)> OnContainerChangeDelegate;
	
	UPROPERTY(BlueprintAssignable)
	FOnContainerChangeSignature OnContainerChange;
	
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags);
	
	UFUNCTION(BlueprintCallable, Category = "Item Container")
	void SetupItemContainer(FGameplayTagContainer ContainerTags);
	
	void AddStartingData();

	UFUNCTION(BlueprintPure, Category = "Item Container")
	bool CanAddItem(const UEISItemInstance* Item) const;

	UFUNCTION(BlueprintPure, Category = "Item Container")
	bool Contains(const UEISItemInstance* Item) const;

	UFUNCTION(BlueprintPure, Category = "Item Container")
	UEISItemInstance* FindFirstStackForItem(const UEISItemInstance* ForItem) const;

	UFUNCTION(BlueprintPure, Category = "Item Container")
	UEISItemInstance* FindItemByDefinition(const UEISItemDefinition* Definition) const;

	UFUNCTION(BlueprintPure, Category = "Item Container")
	UEISItemInstance* FindItemByName(const FName& ScriptName) const;

	UFUNCTION(BlueprintPure, Category = "Item Container")
	UEISItemInstance* FindItemById(int ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Item Container")
	TArray<UEISItemInstance*> GetItems() const { return Items; }

protected:
	virtual void CallAddItem(UEISItemInstance* Item) override;
	virtual void CallLeaveItem(UEISItemInstance* Item) override;
	virtual void CallRemoveItem(UEISItemInstance* Item) override;
	virtual void CallSubtractOrRemoveItem(UEISItemInstance* Item, int Amount) override;
	
	UFUNCTION(BlueprintCallable, Category = "Item Container")
	bool FindAvailablePlace(UEISItemInstance* Item);
	
	UFUNCTION(BlueprintCallable, Category = "Item Container")
	void AddItem(UEISItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category = "Item Container")
	void RemoveItem(UEISItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category = "Item Container")
	bool StackItem(UEISItemInstance* SourceItem, UEISItemInstance* TargetItem);

	UFUNCTION(BlueprintCallable, Category = "Item Container")
	bool SplitItem(UEISItemInstance* Item, int Amount);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Item Container")
	FGameplayTagContainer CategoryTags;

	UPROPERTY(EditDefaultsOnly, Category = "Item Container")
	TArray<TSubclassOf<UEISItemInstance>> StartingData;

	UPROPERTY(EditInstanceOnly, ReplicatedUsing = "OnRep_Items", Category = "Item Container")
	TArray<UEISItemInstance*> Items;

	UFUNCTION()
	void OnRep_Items(TArray<UEISItemInstance*> PrevContainer);
};

