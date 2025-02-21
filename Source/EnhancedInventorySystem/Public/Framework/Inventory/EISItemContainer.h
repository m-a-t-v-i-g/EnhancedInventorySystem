// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EISItem.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "EISItemContainer.generated.h"

class UEISItem;

USTRUCT()
struct FEISItemContainerChangeData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<UEISItem*> AddedItems;

	UPROPERTY()
	TArray<UEISItem*> RemovedItems;

	FEISItemContainerChangeData()
	{
	}
	
	FEISItemContainerChangeData(const TArray<UEISItem*>& NewItems, const TArray<UEISItem*>& OldItems)
	{
		AddedItems = NewItems;
		RemovedItems = OldItems;
	}
};

UCLASS(DisplayName = "Item Container", EditInlineNew, DefaultToInstanced)
class ENHANCEDINVENTORYSYSTEM_API UEISItemContainer : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags);
	
	void AddStartingData();

	UFUNCTION(BlueprintCallable, Category = "EIS|Item Container")
	bool FindAvailablePlace(UEISItem* Item);
	
	UFUNCTION(BlueprintCallable, Category = "EIS|Item Container")
	void AddItem(UEISItem* Item);

	UFUNCTION(BlueprintCallable, Category = "EIS|Item Container")
	void RemoveItem(UEISItem* Item);

	UFUNCTION(BlueprintCallable, Category = "EIS|Item Container")
	bool StackItem(UEISItem* SourceItem, UEISItem* TargetItem);

	UFUNCTION(BlueprintCallable, Category = "EIS|Item Container")
	bool SplitItem(UEISItem* Item, int Amount);

	UFUNCTION(BlueprintPure, Category = "EIS|Item Container")
	bool CanAddItem(const UEISItem* Item) const;

	UFUNCTION(BlueprintPure, Category = "EIS|Item Container")
	bool Contains(const UEISItem* Item) const;

	UFUNCTION(BlueprintPure, Category = "EIS|Item Container")
	UEISItem* FindStackForItem(const UEISItem* ForItem) const;

	UFUNCTION(BlueprintPure, Category = "EIS|Item Container")
	UEISItem* FindItemByDefinition(const UEISItemDefinition* Definition) const;

	UFUNCTION(BlueprintPure, Category = "EIS|Item Container")
	UEISItem* FindItemByName(const FName& ScriptName) const;

	UFUNCTION(BlueprintPure, Category = "EIS|Item Container")
	TArray<UEISItem*> GetItems() const { return Items; }
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Item Container")
	FGameplayTagContainer CategoryTags;

	UPROPERTY(EditDefaultsOnly, Category = "Item Container")
	TArray<UEISItem*> StartingData;

	UPROPERTY(EditInstanceOnly, ReplicatedUsing = "OnRep_Items", Category = "Item Container")
	TArray<UEISItem*> Items;

	UFUNCTION()
	void OnRep_Items(TArray<UEISItem*> PrevContainer);
};

