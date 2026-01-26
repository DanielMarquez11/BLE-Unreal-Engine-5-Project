#include "BLESubsystem.h"

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
}

void UBLESubsystem::Disconnect()
{
}
