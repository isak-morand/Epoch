#pragma once
#include <memory>
#include <nvrhi/nvrhi.h>

namespace Epoch
{
	class Window;
}

namespace Epoch::Rendering
{
	class DeviceManager
	{
	public:
		virtual ~DeviceManager();

		static std::unique_ptr<DeviceManager> Create();

		virtual bool Initialize() = 0;

		nvrhi::IDevice* GetDeviceHandle() { return myNVRHIDevice.Get(); }

	protected:
		DeviceManager();

	protected:
		nvrhi::DeviceHandle myNVRHIDevice;
	};
}