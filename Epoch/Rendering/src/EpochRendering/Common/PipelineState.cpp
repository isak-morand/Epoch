#include "epch.h"
#include "PipelineState.h"
#include "RenderContext.h"
#include "Framebuffer.h"
#include "Shaders/Shader.h"

namespace Epoch::Rendering
{
	namespace Utils
	{
		inline nvrhi::Format GetNVRHIFormat(ShaderDataType type)
		{
			switch (type)
			{
				case ShaderDataType::Float:		return nvrhi::Format::R32_FLOAT;
				case ShaderDataType::Float2:	return nvrhi::Format::RG32_FLOAT;
				case ShaderDataType::Float3:	return nvrhi::Format::RGB32_FLOAT;
				case ShaderDataType::Float4:	return nvrhi::Format::RGBA32_FLOAT;
				case ShaderDataType::Int:		return nvrhi::Format::R8_SINT;
				case ShaderDataType::Int2:		return nvrhi::Format::RG8_SINT;
				case ShaderDataType::Int3:		return nvrhi::Format::RGBA8_SINT;
				case ShaderDataType::Int4:		return nvrhi::Format::RGBA8_SINT;
				case ShaderDataType::UInt:		return nvrhi::Format::R8_UINT;
				case ShaderDataType::UInt2:		return nvrhi::Format::RG8_UINT;
				case ShaderDataType::UInt3:		return nvrhi::Format::RGBA8_UINT;
				case ShaderDataType::UInt4:		return nvrhi::Format::RGBA8_UINT;
			}

			EPOCH_ASSERT(false, "Unknown format!");
			return nvrhi::Format::UNKNOWN;
		}

		inline nvrhi::PrimitiveType GetNVRHIPrimitiveType(PrimitiveTopology aTopology)
		{
			switch (aTopology)
			{
				case PrimitiveTopology::PointList:		return nvrhi::PrimitiveType::PointList;
				case PrimitiveTopology::LineList:		return nvrhi::PrimitiveType::LineList;
				case PrimitiveTopology::TriangleList:	return nvrhi::PrimitiveType::TriangleList;
				case PrimitiveTopology::TriangleStrip:	return nvrhi::PrimitiveType::TriangleStrip;
			}
			EPOCH_ASSERT(false, "Unkown topology!");
			return nvrhi::PrimitiveType::PointList;
		}

		inline nvrhi::ComparisonFunc GetNVRHIDepthCompareOperator(DepthCompareOperator aDepthCompareOperator)
		{
			switch (aDepthCompareOperator)
			{
				case DepthCompareOperator::Never:			return nvrhi::ComparisonFunc::Never;
				case DepthCompareOperator::NotEqual:		return nvrhi::ComparisonFunc::NotEqual;
				case DepthCompareOperator::Less:			return nvrhi::ComparisonFunc::Less;
				case DepthCompareOperator::LessOrEqual:		return nvrhi::ComparisonFunc::LessOrEqual;
				case DepthCompareOperator::Greater:			return nvrhi::ComparisonFunc::Greater;
				case DepthCompareOperator::GreaterOrEqual:	return nvrhi::ComparisonFunc::GreaterOrEqual;
				case DepthCompareOperator::Equal:			return nvrhi::ComparisonFunc::Equal;
				case DepthCompareOperator::Always:			return nvrhi::ComparisonFunc::Always;
			}
			EPOCH_ASSERT(false, "Unkown depth compare operator!");
			return nvrhi::ComparisonFunc::LessOrEqual;
		}

		inline nvrhi::RasterCullMode GetNVRHICullMode(RasterizerCullMode aCullMode)
		{
			switch (aCullMode)
			{
				case RasterizerCullMode::CullBack:		return nvrhi::RasterCullMode::Back;
				case RasterizerCullMode::CullNone:		return nvrhi::RasterCullMode::None;
				case RasterizerCullMode::CullFront:		return nvrhi::RasterCullMode::Front;
			}
			EPOCH_ASSERT(false, "Unkown rasterizer state!");
			return nvrhi::RasterCullMode::Back;
		}
	}

	PipelineState::PipelineState(const PipelineStateSpecification& aSpecification) : mySpecification(aSpecification)
	{
		auto device = RenderContext::Get().DeviceManager->GetDeviceHandle();

		nvrhi::GraphicsPipelineDesc pipelineDesc;

#pragma region Shaders

		if (mySpecification.Shader->HasShader(ShaderStage::Vertex))
		{
			pipelineDesc.VS = mySpecification.Shader->GetShader(ShaderStage::Vertex);
		}
		if (mySpecification.Shader->HasShader(ShaderStage::Geometry))
		{
			pipelineDesc.GS = mySpecification.Shader->GetShader(ShaderStage::Geometry);
		}
		if (mySpecification.Shader->HasShader(ShaderStage::Pixel))
		{
			pipelineDesc.PS = mySpecification.Shader->GetShader(ShaderStage::Pixel);
		}
		//TODO: Add binding layouts when those are implemented

#pragma endregion

#pragma region InputLayout

		uint32_t totalElementCount = 0;
		for (const auto& layout : mySpecification.VertexLayouts)
		{
			totalElementCount += layout.GetElementCount();
		}

		EPOCH_ASSERT(totalElementCount <= nvrhi::c_MaxVertexAttributes, "Total vertex element count for pipeline state '{}' exceeds the maximum!", mySpecification.DebugName);
		nvrhi::static_vector<nvrhi::VertexAttributeDesc, nvrhi::c_MaxVertexAttributes> vertexAttributes;

		uint32_t bufferIndex = 0;
		for (size_t i = 0; i < mySpecification.VertexLayouts.size(); i++)
		{
			const VertexBufferLayout& layout = mySpecification.VertexLayouts[i];

			for (const VertexBufferElement& element : layout)
			{
				nvrhi::VertexAttributeDesc& attributeDesc = vertexAttributes.emplace_back();
				attributeDesc.bufferIndex = bufferIndex;
				attributeDesc.name = element.Name;
				attributeDesc.format = Utils::GetNVRHIFormat(element.Type);
				attributeDesc.offset = element.Offset;
				attributeDesc.elementStride = layout.GetStride();
				attributeDesc.isInstanced = layout.GetInputRate() == ShaderDataInputRate::PerInstance;
			}
			++bufferIndex;
		}
		
		pipelineDesc.inputLayout = device->createInputLayout(vertexAttributes.data(), (uint32_t)vertexAttributes.size(), pipelineDesc.VS);

#pragma endregion

#pragma region Topology

		pipelineDesc.primType = Utils::GetNVRHIPrimitiveType(mySpecification.PrimitiveTopology);

#pragma endregion

#pragma region RasterState

		nvrhi::RasterState& rasterState = pipelineDesc.renderState.rasterState;
		rasterState.cullMode = Utils::GetNVRHICullMode(mySpecification.RasterizerCullMode);
		rasterState.fillMode = mySpecification.Wireframe ? nvrhi::RasterFillMode::Wireframe : nvrhi::RasterFillMode::Solid;

#pragma endregion

#pragma region DepthStencilState

		nvrhi::DepthStencilState& depthStencilState = pipelineDesc.renderState.depthStencilState;
		depthStencilState.depthTestEnable = mySpecification.DepthTest;
		depthStencilState.depthWriteEnable = mySpecification.DepthWrite;
		depthStencilState.depthFunc = Utils::GetNVRHIDepthCompareOperator(mySpecification.DepthCompareOperator);

#pragma endregion

#pragma region BlendState

		// TODO: Add when frambuffer is implemented
		nvrhi::BlendState& blendState = pipelineDesc.renderState.blendState;

#pragma endregion

		myHandle = device->createGraphicsPipeline(pipelineDesc, mySpecification.TargetFramebuffer->GetHandle());
	}

	PipelineState::~PipelineState() = default;
}
