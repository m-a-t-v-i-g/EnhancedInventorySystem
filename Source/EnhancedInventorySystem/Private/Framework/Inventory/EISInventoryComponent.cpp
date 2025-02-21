// Fill out your copyright notice in the Description page of Project Settings.

#include "EISInventoryComponent.h"

UEISInventoryComponent::UEISInventoryComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}
