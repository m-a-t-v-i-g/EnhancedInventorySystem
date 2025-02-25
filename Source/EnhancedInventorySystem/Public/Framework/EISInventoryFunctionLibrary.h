// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EISInventoryFunctionLibrary.generated.h"

class UEISItemContainer;
class UEISItemInstance;

UCLASS()
class ENHANCEDINVENTORYSYSTEM_API UEISInventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Function Library")
	static UEISItemInstance* GenerateItem(UWorld* World, const UEISItemInstance* SourceItem);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Function Library")
	static bool Container_FindAvailablePlace(UEISItemContainer* Container, UEISItemInstance* Item);
	
	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Function Library")
	static void Container_AddItem(UEISItemContainer* Container, UEISItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Function Library")
	static void Container_RemoveItem(UEISItemContainer* Container, UEISItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Function Library")
	static bool Container_StackItem(UEISItemContainer* Container, UEISItemInstance* SourceItem, UEISItemInstance* TargetItem);
	
	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Function Library")
	static void Container_SplitItem(UEISItemContainer* Container, UEISItemInstance* Item, int Amount);

	UFUNCTION(BlueprintCallable, Category = "EIS|Inventory Function Library")
	static void Container_MoveItemToOtherContainer(UEISItemContainer* SourceContainer,
	                                               UEISItemContainer* TargetContainer, UEISItemInstance* Item,
	                                               bool bFullStack = false);

private:
	static int LastItemId;
};
