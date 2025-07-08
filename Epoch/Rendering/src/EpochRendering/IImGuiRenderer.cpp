#include "epch.h"
#include "IImGuiRenderer.h"
#include "ImGui/ImGuiRenderer.h"

namespace Epoch::Rendering
{
	std::unique_ptr<IImGuiRenderer> IImGuiRenderer::Create()
	{
		return std::make_unique<ImGuiRenderer>();
	}

	IImGuiRenderer::IImGuiRenderer() = default;
	IImGuiRenderer::~IImGuiRenderer() = default;
}
