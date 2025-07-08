#include "epch.h"
#include "IRenderer.h"
#include "EpochRendering/Common/Renderer.h"

namespace Epoch::Rendering
{
	std::unique_ptr<IRenderer> IRenderer::Create()
	{
		return std::make_unique<Renderer>();
	}

	IRenderer::IRenderer() = default;
	IRenderer::~IRenderer() = default;
}
