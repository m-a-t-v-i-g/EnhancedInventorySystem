// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EISInventoryFunctionLibrary.generated.h"

class UEISItemContainer;
class UEISEquipmentSlot;
class UEISItemInstance;

UCLASS()
class ENHANCEDINVENTORYSYSTEM_API UEISInventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory Function Library")
	static UEISItemInstance* GenerateItem(UWorld* World, const UEISItemInstance* SourceItem);

	UFUNCTION(BlueprintCallable, Category = "Inventory Function Library|Container")
	static bool Container_FindAvailablePlace(UEISItemContainer* Container, UEISItemInstance* Item);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory Function Library|Container")
	static void Container_AddItem(UEISItemContainer* Container, UEISItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory Function Library|Container")
	static void Container_RemoveItem(UEISItemContainer* Container, UEISItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory Function Library|Container")
	static bool Container_StackItem(UEISItemContainer* Container, UEISItemInstance* SourceItem, UEISItemInstance* TargetItem);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory Function Library|Container")
	static void Container_SplitItem(UEISItemContainer* Container, UEISItemInstance* Item, int Amount);

	UFUNCTION(BlueprintCallable, Category = "Inventory Function Library|Slot")
	static void Slot_EquipItem(UEISEquipmentSlot* EquipmentSlot, UEISItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory Function Library|Slot")
	static void Slot_UnequipItem(UEISEquipmentSlot* EquipmentSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory Function Library|Container")
	static void MoveItemFromContainerToContainer(UEISItemContainer* SourceContainer,
	                                             UEISItemContainer* TargetContainer, UEISItemInstance* Item,
	                                             bool bFullStack = false);

	UFUNCTION(BlueprintCallable, Category = "Inventory Function Library|Container")
	static void MoveItemFromContainerToSlot(UEISItemContainer* SourceContainer,
	                                        UEISEquipmentSlot* TargetSlot, UEISItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory Function Library|Slot")
	static void MoveItemFromSlotToContainer(UEISEquipmentSlot* SourceSlot, UEISItemContainer* TargetContainer);

	UFUNCTION(BlueprintCallable, Category = "Inventory Function Library|Slot")
	static void MoveItemFromSlotToSlot(UEISEquipmentSlot* SourceSlot, UEISEquipmentSlot* TargetSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	static void RemoveItemFromSource(UObject* Source, UEISItemInstance* Item);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	static void SubtractOrRemoveItemFromSource(UObject* Source, UEISItemInstance* Item, int Amount);
	
private:
	static int LastItemId;
};
