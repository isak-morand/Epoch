#pragma once
#include <memory>

namespace Epoch
{
	class Window;
}

namespace Epoch::Rendering
{
	class IImGuiRenderer
	{
	public:
		IImGuiRenderer();
		~IImGuiRenderer();

		static std::unique_ptr<IImGuiRenderer> Create();

		virtual bool Initialize(Window* aWindow) = 0;
		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

	private:

	};
}
