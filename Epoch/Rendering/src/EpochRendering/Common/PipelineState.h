#pragma once
#include <string>
#include <nvrhi/nvrhi.h>
#include "Resources/VertexBufferLayout.h"

namespace Epoch::Rendering
{
	class Shader;
	class Framebuffer;

	enum class PrimitiveTopology { PointList, LineList, TriangleList, TriangleStrip };
	enum class RasterizerCullMode { CullBack, CullNone, CullFront };
	enum class BlendMode { OneZero, Alpha, Additive };
	enum class DepthCompareOperator { Never, NotEqual, Less, LessOrEqual, Greater, GreaterOrEqual, Equal, Always };

	struct PipelineStateSpecification
	{
		std::shared_ptr<Shader> Shader;
		std::shared_ptr<Framebuffer> TargetFramebuffer;

		std::vector<VertexBufferLayout> VertexLayouts;

		PrimitiveTopology PrimitiveTopology = PrimitiveTopology::TriangleList;
		RasterizerCullMode RasterizerCullMode = RasterizerCullMode::CullBack;
		BlendMode BlendMode = BlendMode::OneZero;
		DepthCompareOperator DepthCompareOperator = DepthCompareOperator::LessOrEqual;

		bool Wireframe = false;

		bool DepthTest = true;
		bool DepthWrite = true;

		std::string DebugName = "PipelineState";
	};

	class PipelineState
	{
	public:
		PipelineState() = delete;
		PipelineState(const PipelineStateSpecification& aSpecification);
		~PipelineState();

		nvrhi::GraphicsPipelineHandle GetHandle() const { return myHandle; }

		const std::shared_ptr<Framebuffer>& GetTargetFrameBuffer() const { return mySpecification.TargetFramebuffer; }

	private:
		PipelineStateSpecification mySpecification;
		nvrhi::GraphicsPipelineHandle myHandle;
	};
}
