// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EISItemRepositoryInterface.generated.h"

class UEISItemInstance;

UINTERFACE()
class UEISItemRepositoryInterface : public UInterface
{
	GENERATED_BODY()
};

class ENHANCEDINVENTORYSYSTEM_API IEISItemRepositoryInterface
{
	GENERATED_BODY()

public:
	virtual void CallAddItem(UEISItemInstance* Item);
	
	virtual void CallLeaveItem(UEISItemInstance* Item);
	
	virtual void CallRemoveItem(UEISItemInstance* Item);

	virtual void CallSubtractOrRemoveItem(UEISItemInstance* Item, int Amount);
};
