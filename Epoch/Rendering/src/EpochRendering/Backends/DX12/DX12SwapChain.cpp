#include "DX12SwapChain.h"
#include <EpochCore/Assert.h>
#include <EpochCore/Window/Window.h>
#include "DX12DeviceManager.h"
#include "EpochRendering/Common/RenderContext.h"
#include <nvrhi/d3d12.h>
#include <nvrhi/utils.h>

using namespace Microsoft::WRL;

namespace Epoch::Rendering
{
	bool DX12SwapChain::Initialize(Epoch::Window* aWindow, DeviceManager* aDeviceManager)
	{
		myDeviceManager = aDeviceManager;
		myWidth = aWindow->GetWidth();
		myHeight = aWindow->GetHeight();

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = 2;
		swapChainDesc.Width = myWidth;
		swapChainDesc.Height = myHeight;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		DX12DeviceManager* dm = (DX12DeviceManager*)myDeviceManager;

		ComPtr<IDXGISwapChain1> swapChain1;
		dm->GetFactory()->CreateSwapChainForHwnd
		(
			dm->GetCommandQueue(),
			(HWND)aWindow->GetNativeWindow(),
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain1
		);

		ComPtr<IDXGISwapChain3> swapChain;
		swapChain1.As(&swapChain);

		myFrameIndex = swapChain->GetCurrentBackBufferIndex();

		return true;
	}

	void DX12SwapChain::Resize(uint32_t aWidth, uint32_t aHeight)
	{
		myWidth = aWidth;
		myHeight = aHeight;
	}

	void DX12SwapChain::BeginFrame()
	{
	}

	void DX12SwapChain::EndFrame()
	{
	}

	void DX12SwapChain::Present()
	{
	}

	bool DX12SwapChain::GetBackBuffer()
	{
		DX12DeviceManager* dm = (DX12DeviceManager*)myDeviceManager;

		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = 2;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		ComPtr<ID3D12DescriptorHeap> rtvHeap;
		dm->GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

		auto textureDesc = nvrhi::TextureDesc()
			.setDimension(nvrhi::TextureDimension::Texture2D)
			.setFormat(nvrhi::Format::RGBA8_UNORM)
			.setWidth(myWidth)
			.setHeight(myHeight)
			.setIsRenderTarget(true)
			.setDebugName("Swap Chain Image");

		for (UINT i = 0; i < 2; ++i)
		{
			BackBuffer& backBuffer = myBackBuffers.emplace_back();

			mySwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer.texture));
			backBuffer.textureHandle = RenderContext::Get().deviceHandle->createHandleForNativeTexture(nvrhi::ObjectTypes::D3D12_Resource, nvrhi::Object(myBackBuffers[i].texture.Get()), textureDesc);
		}

		return true;
	}
}
