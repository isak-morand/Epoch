#pragma once
#include "EpochRendering/IImGuiRenderer.h"

namespace Epoch::Rendering
{
	class ImGuiRenderer : public IImGuiRenderer
	{
	public:
		ImGuiRenderer();
		~ImGuiRenderer();

		bool Initialize(Window* aWindow) override;
		void BeginFrame() override;
		void EndFrame() override;

	private:

	};
}
