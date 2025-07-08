#include "epch.h"
#include "DeviceManager.h"

#if defined(USE_DX11)
#include "EpochRendering/Backends/DX11/DX11DeviceManager.h"
#elif defined(USE_DX12)
#include "EpochRendering/Backends/DX12/DX12DeviceManager.h"
#elif defined(USE_VULKAN)
#include "EpochRendering/Backends/Vulkan/VKDeviceManager.h"
#endif

#include "SwapChain.h"

namespace Epoch::Rendering
{
	std::unique_ptr<DeviceManager> DeviceManager::Create()
	{
#if defined(USE_DX11)
		return std::make_unique<DX11DeviceManager>();
#elif defined(USE_DX12)
		return std::make_unique<DX12DeviceManager>();
#elif defined(USE_VULKAN)
		return std::make_unique<VKDeviceManager>();
#else
		return nullptr;
#endif
	}

	DeviceManager::DeviceManager() = default;
	DeviceManager::~DeviceManager() = default;
}