// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "EISInventoryComponent.generated.h"

class UEISItemContainer;
class UEISItemInstance;

UCLASS(DisplayName = "Inventory Component", Abstract, ClassGroup = "EIS", meta = (BlueprintSpawnableComponent))
class ENHANCEDINVENTORYSYSTEM_API UEISInventoryComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:
	UEISInventoryComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Component")
	void FindAvailablePlace(UEISItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Component")
	void AddItem(UEISItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Component")
	void RemoveItem(UEISItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Component")
	void StackItem(UEISItemInstance* SourceItem, UEISItemInstance* TargetItem);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Component")
	void SplitItem(UEISItemInstance* Item, int Amount);
	
	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Component")
	void SetItemContainer(UEISItemContainer* NewItemContainer);

	UFUNCTION(BlueprintPure, Category = "EIS|Inventory Component")
	FORCEINLINE UEISItemContainer* GetItemContainer() const { return ItemContainer; }

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Instanced, Category = "Inventory Component")
	TObjectPtr<UEISItemContainer> ItemContainer;
};
