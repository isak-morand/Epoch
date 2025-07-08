#pragma once
#include "EpochRendering/Common/DeviceManager.h"
#include <wrl/client.h>
#include <nvrhi/nvrhi.h>
#include <nvrhi/d3d11.h>

namespace Epoch::Rendering
{
	class DX11DeviceManager : public DeviceManager
	{
	public:
		DX11DeviceManager() = default;
		~DX11DeviceManager() override;

		bool Initialize() override;

		ID3D11Device* GetDevice() const { return myDevice.Get(); }
		ID3D11DeviceContext* GetContext() const { return myContext.Get(); }

		Microsoft::WRL::ComPtr<IDXGIFactory> GetFactory() const { return myFactory; }

	private:
		Microsoft::WRL::ComPtr<ID3D11Device> myDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> myContext;
		Microsoft::WRL::ComPtr<IDXGIFactory> myFactory;
	};
}
