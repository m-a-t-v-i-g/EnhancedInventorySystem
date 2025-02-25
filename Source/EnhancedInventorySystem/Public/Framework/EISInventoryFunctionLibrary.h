// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EISInventoryFunctionLibrary.generated.h"

class UEISItemContainer;
class UEISItem;

namespace FInventorySystemCore
{
	
}

UCLASS()
class ENHANCEDINVENTORYSYSTEM_API UEISInventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Function Library")
	static UEISItem* GenerateItem(UWorld* World, const UEISItem* SourceItem);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Function Library")
	static bool Container_FindAvailablePlace(UEISItemContainer* Container, UEISItem* Item);
	
	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Function Library")
	static void Container_AddItem(UEISItemContainer* Container, UEISItem* Item);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Function Library")
	static void Container_RemoveItem(UEISItemContainer* Container, UEISItem* Item);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Function Library")
	static bool Container_StackItem(UEISItemContainer* Container, UEISItem* SourceItem, UEISItem* TargetItem);
	
	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Function Library")
	static void Container_SplitItem(UEISItemContainer* Container, UEISItem* Item, int Amount);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Function Library")
	static void Container_MoveItemToOtherContainer(UEISItemContainer* SourceContainer,
	                                               UEISItemContainer* TargetContainer, UEISItem* Item,
	                                               bool bFullStack = false);
	
private:
	static int LastItemId;
};
