// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BLEDevice.generated.h"

/**
 * 
 */
UCLASS()
class BLUETHOOTHHEARTRATE_API UBLEDevice : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable,Category="BLE")
	static void StartBLEScan();
	
};
