// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "EISEquipmentComponent.generated.h"

class UEISEquipmentSlot;
class UEISItemInstance;

UCLASS(DisplayName = "Equipment Component", Abstract)
class ENHANCEDINVENTORYSYSTEM_API UEISEquipmentComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:
	UEISEquipmentComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Equipment Component")
	void EquipSlot(const FString& SlotName, UEISItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, Category = "Equipment Component")
	void UnequipSlot(const FString& SlotName);

	UFUNCTION(BlueprintPure, Category = "Equipment Component")
	UEISEquipmentSlot* FindEquipmentSlotByName(const FString& SlotName) const;

	UFUNCTION(BlueprintPure, Category = "Equipment Component")
	TArray<UEISEquipmentSlot*> GetEquipmentSlots() const { return EquipmentSlots; }
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Instanced, Category = "Equipment Component")
	TArray<UEISEquipmentSlot*> EquipmentSlots;
};
