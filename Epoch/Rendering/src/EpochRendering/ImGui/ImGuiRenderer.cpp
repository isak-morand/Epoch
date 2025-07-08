#include "epch.h"
#include "ImGuiRenderer.h"
#include <imgui/imgui.h>

namespace Epoch::Rendering
{
	ImGuiRenderer::ImGuiRenderer() = default;
	ImGuiRenderer::~ImGuiRenderer() = default;

	bool ImGuiRenderer::Initialize(Window* aWindow)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		return true;
	}

	void ImGuiRenderer::BeginFrame()
	{
	}

	void ImGuiRenderer::EndFrame()
	{
	}
}
