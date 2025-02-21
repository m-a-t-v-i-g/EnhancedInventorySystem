// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EISInventoryFunctionLibrary.generated.h"

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
	static UEISItem* GenerateItem(UWorld* World, const UEISItem* Item);

private:
	static int LastItemId;
};
