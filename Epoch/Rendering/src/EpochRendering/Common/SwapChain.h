#pragma once
#include <memory>
#include <CommonUtilities/Color.h>

namespace nvrhi
{
	class IFramebuffer;
}

namespace Epoch::Rendering
{
	class DeviceManager;

	class SwapChain
	{
	public:
		virtual ~SwapChain();

		static std::unique_ptr<SwapChain> Create();

		virtual bool Initialize(void* aWindow, uint32_t aWidth, uint32_t aHeight) = 0;
		virtual void Resize(uint32_t aWidth, uint32_t aHeight) = 0;

		uint32_t GetWidth() const { return myWidth; }
		uint32_t GetHeight() const { return myHeight; }

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual nvrhi::IFramebuffer* GetFrameBuffer() = 0;

	protected:
		SwapChain();
		virtual void Present() = 0;

	protected:
		uint32_t myWidth = 0, myHeight = 0;
	};
}
