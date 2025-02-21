// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EISInventoryComponent.generated.h"

class UEISItemContainer;

UCLASS(DisplayName = "Inventory Component", ClassGroup = "Enhanced Inventory System",
	meta = (BlueprintSpawnableComponent))
class ENHANCEDINVENTORYSYSTEM_API UEISInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEISInventoryComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "EIS|Inventory Component")
	FORCEINLINE UEISItemContainer* GetItemContainer() const { return ItemContainer; }

protected:
	UPROPERTY(EditAnywhere, Instanced, Category = "Inventory Component")
	TObjectPtr<UEISItemContainer> ItemContainer;
};
