#include "epch.h"
#include "DX11SwapChain.h"
#include "DX11DeviceManager.h"
#include "EpochRendering/Common/RenderContext.h"

using namespace Microsoft::WRL;

namespace Epoch::Rendering
{
	DX11SwapChain::~DX11SwapChain() {}

	bool DX11SwapChain::Initialize(void* aWindow, uint32_t aWidth, uint32_t aHeight)
	{
		myWidth = aWidth;
		myHeight = aHeight;

		DXGI_SWAP_CHAIN_DESC swapDesc = {};
		swapDesc.BufferCount = 2;
		swapDesc.BufferDesc.Width = myWidth;
		swapDesc.BufferDesc.Height = myHeight;
		swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapDesc.OutputWindow = (HWND)aWindow;
		swapDesc.SampleDesc.Count = 1;
		swapDesc.Windowed = TRUE;
		swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

		DX11DeviceManager* dm = (DX11DeviceManager*)RenderContext::Get().DeviceManager;
		HRESULT hr = dm->GetFactory()->CreateSwapChain(dm->GetDevice(), &swapDesc, &mySwapChain);
		if (FAILED(hr)) return false;

		GetBackBuffer();

		return true;
	}

	void DX11SwapChain::Resize(uint32_t aWidth, uint32_t aHeight)
	{
		myWidth = aWidth;
		myHeight = aHeight;

		myBackBuffer = BackBuffer();
		mySwapChainFrameBuffer = nvrhi::FramebufferHandle();

		HRESULT result = mySwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
		EPOCH_ASSERT(SUCCEEDED(result), "Failed to resize the swap chain!");

		GetBackBuffer();
	}

	void DX11SwapChain::BeginFrame()
	{
	}

	void DX11SwapChain::EndFrame()
	{
		Present();
	}

	nvrhi::IFramebuffer* DX11SwapChain::GetFrameBuffer()
	{
		return mySwapChainFrameBuffer.Get();
	}

	void DX11SwapChain::Present()
	{
		mySwapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
	}

	bool DX11SwapChain::GetBackBuffer()
	{
		HRESULT result = mySwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &myBackBuffer.Texture);
		if (FAILED(result)) return false;

		auto textureDesc = nvrhi::TextureDesc()
			.setDimension(nvrhi::TextureDimension::Texture2D)
			.setFormat(nvrhi::Format::RGBA8_UNORM)
			.setWidth(myWidth)
			.setHeight(myHeight)
			.setIsRenderTarget(true)
			.setDebugName("Swap Chain Image");

		myBackBuffer.TextureHandle = RenderContext::Get().DeviceManager->GetDeviceHandle()->createHandleForNativeTexture(nvrhi::ObjectTypes::D3D11_Resource, nvrhi::Object(myBackBuffer.Texture.Get()), textureDesc);

		auto depthtextureDesc = nvrhi::TextureDesc()
			.setDimension(nvrhi::TextureDimension::Texture2D)
			.setFormat(nvrhi::Format::D32)
			.setWidth(myWidth)
			.setHeight(myHeight)
			.setIsRenderTarget(true)
			.setIsTypeless(true)
			.setDebugName("Swap Chain Depth Image");

		myBackBuffer.DepthTextureHandle = RenderContext::Get().DeviceManager->GetDeviceHandle()->createTexture(depthtextureDesc);

		mySwapChainFrameBuffer = RenderContext::Get().DeviceManager->GetDeviceHandle()->createFramebuffer(nvrhi::FramebufferDesc().addColorAttachment(myBackBuffer.TextureHandle).setDepthAttachment(myBackBuffer.DepthTextureHandle));

		return true;
	}
}
