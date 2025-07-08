#pragma once
#include "EpochRendering/Common/DeviceManager.h"
#include <wrl/client.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <nvrhi/nvrhi.h>

namespace Epoch::Rendering
{
	class DX12DeviceManager : public DeviceManager
	{
	public:
		DX12DeviceManager() = default;

		bool Initialize() override;

		void BeginFrame() override;
		void EndFrame() override;

		ID3D12Device* GetDevice() const { return myDevice.Get(); }
		ID3D12CommandQueue* GetCommandQueue() const { return myCommandQueue.Get(); }

		Microsoft::WRL::ComPtr<IDXGIFactory4> GetFactory() const { return myFactory; }

	private:
		Microsoft::WRL::ComPtr<ID3D12Device> myDevice;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> myCommandQueue;
		Microsoft::WRL::ComPtr<IDXGIFactory4> myFactory;
		//Microsoft::WRL::ComPtr<ID3D11DeviceContext> myContext;
	};
}
