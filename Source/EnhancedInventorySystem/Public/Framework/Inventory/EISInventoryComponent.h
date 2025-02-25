// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "EISInventoryComponent.generated.h"

class UEISItemContainer;
class UEISItem;

UCLASS(DisplayName = "Inventory Component", ClassGroup = "Enhanced Inventory System",
	meta = (BlueprintSpawnableComponent))
class ENHANCEDINVENTORYSYSTEM_API UEISInventoryComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:
	UEISInventoryComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Component")
	void FindAvailablePlace(UEISItem* Item);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Component")
	void AddItem(UEISItem* Item);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Component")
	void RemoveItem(UEISItem* Item);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Component")
	void StackItem(UEISItem* SourceItem, UEISItem* TargetItem);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Component")
	void SplitItem(UEISItem* Item, int Amount);
	
	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Component")
	void SetItemContainer(UEISItemContainer* NewItemContainer);

	UFUNCTION(BlueprintPure, Category = "EIS|Inventory Component")
	FORCEINLINE UEISItemContainer* GetItemContainer() const { return ItemContainer; }

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Instanced, Category = "Inventory Component")
	TObjectPtr<UEISItemContainer> ItemContainer;
};
