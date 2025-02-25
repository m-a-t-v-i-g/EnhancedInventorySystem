// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "EISItem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmountChangeSignature, int, NewAmount, int, PrevAmount);

USTRUCT(DisplayName = "Item Instance Data", BlueprintType, Blueprintable)
struct FEISItemInstanceData
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int ItemId = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "1"))
	int Amount = 1;
};

UCLASS(DisplayName = "Item Definition")
class ENHANCEDINVENTORYSYSTEM_API UEISItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Class")
	FName ScriptName;

	UPROPERTY(EditAnywhere, Category = "Class")
	FGameplayTagContainer Tags;
	
	UPROPERTY(EditAnywhere, Category = "Properties")
	bool bStackable = false;
	
	UPROPERTY(EditAnywhere, Category = "Properties", meta = (EditCondition = "bStackable", ClampMin = "1"))
	int StackAmount = 1;
};

UCLASS(DisplayName = "Item Component", BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced, Within = "EISItem")
class ENHANCEDINVENTORYSYSTEM_API UEISItemComponent : public UObject
{
	GENERATED_BODY()
};

UCLASS(DisplayName = "Item", BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class ENHANCEDINVENTORYSYSTEM_API UEISItem : public UObject
{
	GENERATED_BODY()

public:
	UEISItem(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable)
	FOnAmountChangeSignature OnAmountChange;
	
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags);

#pragma region Definition
	
	UFUNCTION(BlueprintPure, Category = "EIS|Item|Definition")
	const UEISItemDefinition* GetDefinition() const;

	UFUNCTION(BlueprintPure, Category = "EIS|Item|Definition")
	FORCEINLINE FName GetScriptName() const;
	
	UFUNCTION(BlueprintPure, Category = "EIS|Item|Definition")
	const FGameplayTagContainer& GetTags() const;
	
	UFUNCTION(BlueprintPure, Category = "EIS|Item|Definition")
	FORCEINLINE bool IsStackable() const;

	UFUNCTION(BlueprintPure, Category = "EIS|Item|Definition")
	FORCEINLINE int GetStackAmount() const;

#pragma endregion Definition

#pragma region Amount
	
	UFUNCTION(BlueprintCallable, Category = "EIS|Item|Amount")
	void SetAmount(int InAmount);
	
	UFUNCTION(BlueprintCallable, Category = "EIS|Item|Amount")
	int AddAmount(int InAmount);
	
	UFUNCTION(BlueprintCallable, Category = "EIS|Item|Amount")
	int RemoveAmount(int InAmount);
	
	UFUNCTION(BlueprintPure, Category = "EIS|Item|Amount")
	int GetAmount() const { return InstanceData.Amount; }
	
#pragma endregion Amount
	
#pragma region Components

	UFUNCTION(BlueprintPure, Category = "EIS|Item|Components")
	TArray<UEISItemComponent*> GetComponents() const { return Components; }
	
#pragma endregion Components

	void OnCreate(int InItemId, const UEISItem* SourceItem);

	UFUNCTION(BlueprintImplementableEvent)
	void K2_OnCreate(const UEISItem* SourceItem);
	
	UFUNCTION(BlueprintPure, Category = "EIS|Item")
	virtual bool CanStackWith(const UEISItem* OtherItem) const;

	UFUNCTION(BlueprintPure, Category = "EIS|Item")
	virtual bool IsCorrespondsTo(const UEISItem* OtherItem) const;

protected:
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Item")
	FEISItemInstanceData InstanceData;
	
private:
	UPROPERTY(EditAnywhere, Category = "Item")
	TArray<UEISItemComponent*> Components;
	
	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<UEISItemDefinition> ItemDefinition;
};
