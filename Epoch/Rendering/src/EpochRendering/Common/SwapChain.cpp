#include "epch.h"
#include "SwapChain.h"

#if defined(USE_DX11)
#include "EpochRendering/Backends/DX11/DX11SwapChain.h"
#elif defined(USE_DX12)
#include "EpochRendering/Backends/DX12/DX12SwapChain.h"
#elif defined(USE_VULKAN)
#include "EpochRendering/Backends/Vulkan/VKSwapChain.h"
#endif

namespace Epoch::Rendering
{
	std::unique_ptr<SwapChain> Epoch::Rendering::SwapChain::Create()
	{
#if defined(USE_DX11)
		return std::make_unique<DX11SwapChain>();
#elif defined(USE_DX12)
		return std::make_unique<DX12SwapChain>();
#elif defined(USE_VULKAN)
		return std::make_unique<VKSwapChain>();
#else
		return nullptr;
#endif
	}

	SwapChain::SwapChain() = default;
	SwapChain::~SwapChain() = default;
}
