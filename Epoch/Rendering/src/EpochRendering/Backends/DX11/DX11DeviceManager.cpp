#include "epch.h"
#include "DX11DeviceManager.h"
#include "EpochRendering/Common/NVRHIMessageCallback.h"

using namespace Microsoft::WRL;

namespace Epoch::Rendering
{
	DX11DeviceManager::~DX11DeviceManager() {}

	bool DX11DeviceManager::Initialize()
	{
		UINT creationFlags = 0;
#ifdef _DEBUG
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0
		};

		HRESULT hr = D3D11CreateDevice
		(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			creationFlags,
			featureLevels,
			ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION,
			&myDevice,
			nullptr,
			&myContext
		);

		if (FAILED(hr)) return false;

		nvrhi::d3d11::DeviceDesc deviceDesc;
		deviceDesc.messageCallback = &DefaultMessageCallback::GetInstance();
		deviceDesc.context = myContext.Get();

		myNVRHIDevice = nvrhi::d3d11::createDevice(deviceDesc);

		ComPtr<IDXGIDevice> dxgiDevice;

		hr = myDevice.As(&dxgiDevice);
		if (FAILED(hr)) return false;

		ComPtr<IDXGIAdapter> adapter;
		hr = dxgiDevice->GetAdapter(&adapter);
		if (FAILED(hr)) return false;

		hr = adapter->GetParent(__uuidof(IDXGIFactory), &myFactory);
		if (FAILED(hr)) return false;

		return true;
	}
}