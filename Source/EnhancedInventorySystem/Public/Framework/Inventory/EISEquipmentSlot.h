// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EISItemRepositoryInterface.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "EISEquipmentSlot.generated.h"

class UEISInventoryFunctionLibrary;
class UEISItemInstance;

USTRUCT()
struct FEISEquipmentSlotChangeData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FString SlotName;

	UPROPERTY()
	UEISItemInstance* SlotItem = nullptr;

	UPROPERTY()
	bool bIsEquipped = false;
	
	FEISEquipmentSlotChangeData()
	{
	}
	
	FEISEquipmentSlotChangeData(const FString& Name, UEISItemInstance* Item, bool bEquipped) : SlotName(Name),
		SlotItem(Item), bIsEquipped(bEquipped)
	{
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentSlotChangeSignature, const FEISEquipmentSlotChangeData&,
                                            EquipmentSlotChangeData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentSlotAvailabilitySignature, bool, Available);

UCLASS(DisplayName = "Equipment Slot", Abstract, EditInlineNew, DefaultToInstanced)
class ENHANCEDINVENTORYSYSTEM_API UEISEquipmentSlot : public UObject, public IEISItemRepositoryInterface
{
	GENERATED_BODY()
	
	friend UEISInventoryFunctionLibrary;
	
public:
	TMulticastDelegate<void(const FEISEquipmentSlotChangeData&)> OnEquipmentSlotChangeDelegate;
	
	UPROPERTY(BlueprintAssignable)
	FOnEquipmentSlotChangeSignature OnEquipmentSlotChange;

	UPROPERTY(BlueprintAssignable)
	FOnEquipmentSlotAvailabilitySignature OnAvailabilityChange;
	
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags);

	UFUNCTION(BlueprintCallable, Category = "Equipment Slot")
	void SetupEquipmentSlot(FString InSlotName, FGameplayTagContainer InSlotTags);
	
	void AddStartingData();

	UFUNCTION(BlueprintPure, Category = "Equipment Slot")
	bool CanEquipItem(const UEISItemInstance* Item) const;
	
	UFUNCTION(BlueprintPure, Category = "Equipment Slot")
	bool IsEquipped() const { return ItemInstance != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Equipment Slot")
	bool IsAvailable() const { return bAvailable; }
	
	UFUNCTION(BlueprintPure, Category = "Equipment Slot")
	const FString& GetSlotName() const { return SlotName; }

	UFUNCTION(BlueprintPure, Category = "Equipment Slot")
	UEISItemInstance* GetItemInstance() const { return ItemInstance; }

protected:
	virtual void CallRemoveItem(UEISItemInstance* Item) override;
	
	UFUNCTION(BlueprintCallable, Category = "Equipment Slot")
	void EquipSlot(UEISItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, Category = "Equipment Slot")
	void UnequipSlot();

	UFUNCTION(BlueprintCallable, Category = "Equipment Slot")
	void SetAvailability(bool bInAvailability);

private:
	UPROPERTY(EditAnywhere, Category = "Equipment Slot")
	FString SlotName = "Default";
	
	UPROPERTY(EditAnywhere, Category = "Equipment Slot")
	FGameplayTagContainer CategoryTags;
	
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = "OnRep_ItemInstance", Category = "Equipment Slot")
	TObjectPtr<UEISItemInstance> ItemInstance;

	UPROPERTY(EditInstanceOnly, ReplicatedUsing = "OnRep_Availability", Category = "Equipment Slot")
	bool bAvailable = true;

	UFUNCTION()
	void OnRep_ItemInstance(UEISItemInstance* PrevItem);

	UFUNCTION()
	void OnRep_Availability();
};
