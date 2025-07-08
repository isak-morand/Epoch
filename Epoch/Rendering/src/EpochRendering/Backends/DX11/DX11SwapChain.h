#pragma once
#include "EpochRendering/Common/SwapChain.h"
#include <wrl/client.h>
#include <nvrhi/nvrhi.h>
#include <nvrhi/d3d11.h>

namespace Epoch
{
	class Window;
}

namespace Epoch::Rendering
{
	class DeviceManager;

	class DX11SwapChain : public SwapChain
	{
	public:
		DX11SwapChain() = default;
		~DX11SwapChain() override;

		bool Initialize(void* aWindow, uint32_t aWidth, uint32_t aHeight) override;
		void Resize(uint32_t aWidth, uint32_t aHeight) override;

		void BeginFrame() override;
		void EndFrame() override;

		IDXGISwapChain* GetSwapChain() const { return mySwapChain.Get(); }

		nvrhi::IFramebuffer* GetFrameBuffer() override;

	protected:
		void Present() override;

	private:
		bool GetBackBuffer();

	private:
		Microsoft::WRL::ComPtr<IDXGISwapChain> mySwapChain;

		struct BackBuffer
		{
			Microsoft::WRL::ComPtr<ID3D11Resource> Texture = nullptr;
			nvrhi::TextureHandle TextureHandle;
			nvrhi::TextureHandle DepthTextureHandle;
		} myBackBuffer;

		nvrhi::FramebufferHandle mySwapChainFrameBuffer;
	};
}
