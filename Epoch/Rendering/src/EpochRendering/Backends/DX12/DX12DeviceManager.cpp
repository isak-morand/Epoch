#include "DX12DeviceManager.h"
#include "EpochRendering/Common/NVRHIMessageCallback.h"
#include <nvrhi/d3d12.h>
#include <dxgi1_4.h>

using namespace Microsoft::WRL;

namespace Epoch::Rendering
{
	bool DX12DeviceManager::Initialize()
	{
		{
#ifdef _DEBUG
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();
			}
#endif

			ComPtr<IDXGIFactory4> dxgiFactory;
			CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

			ComPtr<IDXGIAdapter1> hardwareAdapter;
			for (UINT adapterIndex = 0; dxgiFactory->EnumAdapters1(adapterIndex, &hardwareAdapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				hardwareAdapter->GetDesc1(&desc);
				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

				if (SUCCEEDED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&myDevice))))
				{
					break;
				}
			}

			D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

			myDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&myCommandQueue));
		}



		return true;
	}

	void DX12DeviceManager::BeginFrame()
	{
	}

	void DX12DeviceManager::EndFrame()
	{
	}
}
