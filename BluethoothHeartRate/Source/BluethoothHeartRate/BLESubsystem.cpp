#include "BLESubsystem.h"

#include <iostream>

#include "Async/Async.h"

#if PLATFORM_WINDOWS
	#include "Windows/AllowWindowsPlatformTypes.h"
	#include <winrt/Windows.Foundation.h>
	#include <winrt/Windows.Devices.Bluetooth.h>
	#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
	#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
	#include <winrt/Windows.Storage.Streams.h>
	#include "Windows/HideWindowsPlatformTypes.h"
	#include <winrt/Windows.Foundation.Collections.h>

	#include <mutex>
#endif

#if PLATFORM_WINDOWS

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Devices::Bluetooth;
using namespace winrt::Windows::Devices::Bluetooth::Advertisement;
using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace winrt::Windows::Storage::Streams;

static void EnsureWinRT()
{
	static std::once_flag Once;
	std::call_once(Once, []()
	{
		init_apartment(winrt::apartment_type::single_threaded);
	});
}

// 16-bit SIG UUID -> full BLE UUID: 0000xxxx-0000-1000-8000-00805F9B34FB
static winrt::guid GuidFrom16Bit(uint16 ShortUuid)
{
	return winrt::guid{ 0x00000000u | ShortUuid, 0x0000, 0x1000, {0x80,0x00,0x00,0x80,0x5F,0x9B,0x34,0xFB} };
}

struct UBLESubsystem::FImpl
{
	BluetoothLEAdvertisementWatcher Watcher{ nullptr };
	winrt::event_token ReceivedToken{};

	BluetoothLEDevice Device{ nullptr };

	std::atomic<bool> bScanning{ false };
	std::atomic<bool> bConnected{ false };
	uint64 ConnectedAddressU64 = 0;

	void ResetConnection()
	{
		Device = nullptr;
		bConnected = false;
		ConnectedAddressU64 = 0;
	}
};

void UBLESubsystem::FImplDeleter::operator()(FImpl* Ptr) const
{
	delete Ptr;
}

void UBLESubsystem::OnAdverReceived(
	BluetoothLEAdvertisementWatcher watcher,
	BluetoothLEAdvertisementReceivedEventArgs eventArgs)
{
	if (G_TargetDevice != 0) return;

	G_TargetDevice = 232406127976466;
	GEngine->AddOnScreenDebugMessage(
	-1,
	2.0f,
	FColor::Red,
	FString::Printf(TEXT("Connecting to device: %lld"), G_TargetDevice)
);
}

#endif // PLATFORM_WINDOWS

// Keep out-of-line because of Pimpl + UHT TU rules
UBLESubsystem::~UBLESubsystem() = default;

void UBLESubsystem::StartScan()
{
#if PLATFORM_WINDOWS
	EnsureWinRT();

	if (!Impl)
	{
		Impl = TUniquePtr<FImpl, FImplDeleter>(new FImpl());
	}

	if (Impl->bScanning)
	{
		return;
	}

	Impl->Watcher = BluetoothLEAdvertisementWatcher{};
	Impl->Watcher.ScanningMode(BluetoothLEScanningMode::Active);
	BluetoothLEAdvertisementFilter Filter;
	Filter.Advertisement().ServiceUuids().Append(GuidFrom16Bit(0x180D));
	Impl->Watcher.AdvertisementFilter(Filter);
	Impl->ReceivedToken = Impl->Watcher.Received([this](BluetoothLEAdvertisementWatcher const&, BluetoothLEAdvertisementReceivedEventArgs const& Args)
	{
		const uint64 Address = Args.BluetoothAddress();
		const int32 RSSI = Args.RawSignalStrengthInDBm();
		FString Name;
        			try { Name = FString(Args.Advertisement().LocalName().c_str()); }
        			catch (...) { Name = TEXT(""); }
        
        			AsyncTask(ENamedThreads::GameThread, [this, Address, Name, RSSI]()
        			{
        				OnDeviceFound.Broadcast(static_cast<int64>(Address), Name, RSSI);
        				ConnectedAddress = Address;
        			});
	});
	Impl->Watcher.Stopped([this](BluetoothLEAdvertisementWatcher const&, BluetoothLEAdvertisementWatcherStoppedEventArgs const&)
	{
		if (Impl)
		{
			Impl->bScanning = false;
		}
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			bIsScanning = false;
		});
	});
	Impl->bScanning = true;
	bIsScanning = true;
	Impl->Watcher.Start();
#else
	OnError.Broadcast(TEXT("BLE is only implemented for Windows"));
#endif
	
}

void UBLESubsystem::StopScan()
{
}

void UBLESubsystem::Connect(int64 BluetoothAddress)
{
#if PLATFORM_WINDOWS
	EnsureWinRT();

	if (!Impl)
	{
		OnError.Broadcast(TEXT("BLE subsystem not initialized"));
		return;
	}

	if (Impl->bConnected)
	{
		return;
	}

	const uint64 Address = static_cast<uint64>(BluetoothAddress);

	// Stop scanning first (best practice)
	if (Impl->bScanning)
	{
		Impl->Watcher.Stop();
		Impl->bScanning = false;
		bIsScanning = false;
	}

	// Run WinRT async on background thread
	Async(EAsyncExecution::Thread, [this, Address]()
	{
		try
		{
			auto DeviceOp = BluetoothLEDevice::FromBluetoothAddressAsync(Address);
			BluetoothLEDevice Device = DeviceOp.get(); // blocks THIS worker thread, not game thread

			if (!Device)
			{
				AsyncTask(ENamedThreads::GameThread, [this]()
				{
					OnError.Broadcast(TEXT("Failed to connect to BLE device"));
				});
				return;
			}

			Impl->Device = Device;
			Impl->bConnected = true;
			Impl->ConnectedAddressU64 = Address;
			
			FString DeviceName = FString(Device.Name().c_str());
			if (DeviceName.IsEmpty())
			{
				DeviceName = TEXT("Unknown BLE Device");
			}
			AsyncTask(ENamedThreads::GameThread, [this, DeviceName, Address]()
			{
				OnConnected.Broadcast(static_cast<int64>(Address));
			});
		}
		catch (winrt::hresult_error const& Error)
		{
			const FString ErrorMsg = FString::Printf(
				TEXT("BLE connect error: %s"),
				Error.message().c_str()
			);

			AsyncTask(ENamedThreads::GameThread, [this, ErrorMsg]()
			{
				OnError.Broadcast(ErrorMsg);
			});
		}
	});
#else
	OnError.Broadcast(TEXT("BLE is only implemented for Windows"));
#endif
}


void UBLESubsystem::Disconnect()
{
#if PLATFORM_WINDOWS
	if (!Impl || !Impl->bConnected)
	{
		return;
	}

	Impl->ResetConnection();

	OnDisconnected.Broadcast(ConnectedAddress);
#endif
}
