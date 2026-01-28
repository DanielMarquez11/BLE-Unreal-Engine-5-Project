// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <winrt/impl/Windows.Devices.Bluetooth.2.h>
#include <winrt/impl/Windows.Devices.Bluetooth.Advertisement.2.h>
#include <winrt/impl/Windows.Devices.Bluetooth.GenericAttributeProfile.2.h>

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BLESubsystem.generated.h"

// Address is int64 because Blueprint does NOT support uint64 in UE 5.7.2.
// Internally you can cast to uint64 for WinRT.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FBLEOnDeviceFound, int64, BluetoothAddress, const FString&, Name, int32, RSSI);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBLEOnConnected, int64, BluetoothAddress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBLEOnDisconnected, int64, BluetoothAddress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBLEOnError, const FString&, ErrorMessage);

using winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic;
using winrt::Windows::Devices::Bluetooth::BluetoothLEDevice;

/**
 * 
 */
UCLASS()
class BLUETHOOTHHEARTRATE_API UBLESubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Out-of-line destructor recommended because we use a Pimpl.
	virtual ~UBLESubsystem() override;

	/** Start scanning for BLE devices (optionally filter in the .cpp). */
	UFUNCTION(BlueprintCallable, Category="BLE")
	void StartScan();

	/** Stop scanning. */
	UFUNCTION(BlueprintCallable, Category="BLE")
	void StopScan();

	/** Connect to a device by Bluetooth address received from OnDeviceFound. */
	UFUNCTION(BlueprintCallable, Category="BLE")
	void Connect(int64 BluetoothAddress);

	/** Disconnect from current device (if connected). */
	UFUNCTION(BlueprintCallable, Category="BLE")
	void Disconnect();

	UFUNCTION(BlueprintPure, Category="BLE")
	bool IsScanning() const { return bIsScanning; }

	UFUNCTION(BlueprintPure, Category="BLE")
	bool IsConnected() const { return bIsConnected; }

	UFUNCTION(BlueprintPure, Category="BLE")
	int64 GetConnectedAddress() const { return ConnectedAddress; }

	UPROPERTY(BlueprintAssignable, Category="BLE")
	FBLEOnDeviceFound OnDeviceFound;

	UPROPERTY(BlueprintAssignable, Category="BLE")
	FBLEOnConnected OnConnected;

	UPROPERTY(BlueprintAssignable, Category="BLE")
	FBLEOnDisconnected OnDisconnected;

	UPROPERTY(BlueprintAssignable, Category="BLE")
	FBLEOnError OnError;

private:
	// Keep simple state mirrored for Blueprint queries.
	UPROPERTY(Transient)
	bool bIsScanning = false;

	UPROPERTY(Transient)
	bool bIsConnected = false;

	UPROPERTY(Transient)
	int64 ConnectedAddress = 0;

	// Pimpl to keep WinRT headers out of this .h
	struct FImpl;

	// Custom deleter: define operator() in the .cpp AFTER FImpl is defined
	// to avoid "deletion of pointer to incomplete type" (UHT constructor TU issue).
	struct FImplDeleter
	{
		void operator()(FImpl* Ptr) const;
	};

	TUniquePtr<FImpl, FImplDeleter> Impl;


public:
	BluetoothLEDevice* G_Device{ nullptr };
	GattCharacteristic* G_HeartRateChar{nullptr};
	uint64_t G_TargetDevice = 0;
	std::atomic<uint8_t> G_LatestHeartRate = 0;

	UFUNCTION()
	auto OnAdverReceived(winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher watcher,
	                     winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementReceivedEventArgs
	                     eventArgs) -> void;
};
