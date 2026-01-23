// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include <windows.devices.bluetooth.advertisement.h>

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
	uint64_t G_TargetDevice = 0;
	ABI::Windows::Devices::Bluetooth::BluetoothLEDevice* G_Device{ nullptr };
	ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic* G_HeartRateChar{ nullptr };
	std::atomic<uint8_t> G_LatestHeartRate = 0;

	SOCKET S;
	void RunTCPServer();
	
	
};
