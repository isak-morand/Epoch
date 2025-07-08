#pragma once
#include "EpochRendering/Common/SwapChain.h"
#include <wrl/client.h>
#include <dxgi.h>
#include <d3d11.h>
#include <nvrhi/nvrhi.h> //TEMP (maybe)
#include <dxgi1_4.h>

namespace Epoch
{
	class Window;
}

namespace Epoch::Rendering
{
	class DeviceManager;

	class DX12SwapChain : public SwapChain
	{
	public:
		DX12SwapChain() = default;
		~DX12SwapChain() override = default;

		bool Initialize(Epoch::Window* aWindow, DeviceManager* aDeviceManager) override;
		void Resize(uint32_t aWidth, uint32_t aHeight) override;

		void BeginFrame() override;
		void EndFrame() override;

		IDXGISwapChain1* GetSwapChain() const { return mySwapChain.Get(); }

	protected:
		void Present() override;

	private:
		bool GetBackBuffer();

	private:
		DeviceManager* myDeviceManager = nullptr;
		Microsoft::WRL::ComPtr<IDXGISwapChain1> mySwapChain;

		UINT myFrameIndex = 0u;

		struct BackBuffer
		{
			Microsoft::WRL::ComPtr<ID3D11Resource> texture = nullptr;
			nvrhi::TextureHandle textureHandle;
		};

		std::vector<BackBuffer> myBackBuffers;

		std::vector<nvrhi::FramebufferHandle> mySwapChainFrameBuffer;

		nvrhi::CommandListHandle myCommandList;
	};
}
