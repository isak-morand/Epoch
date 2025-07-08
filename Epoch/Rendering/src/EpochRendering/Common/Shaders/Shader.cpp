#include "epch.h"
#include "Shader.h"
#include "EpochRendering/Common/RenderContext.h"

namespace Epoch::Rendering
{
	namespace Utils
	{
		inline nvrhi::ShaderType NVRHIShaderStage(ShaderStage aStage)
		{
			switch (aStage)
			{
				case Rendering::ShaderStage::Vertex:		return nvrhi::ShaderType::Vertex;
				case Rendering::ShaderStage::Geometry:	return nvrhi::ShaderType::Geometry;
				case Rendering::ShaderStage::Pixel:		return nvrhi::ShaderType::Pixel;
				case Rendering::ShaderStage::Compute:	return nvrhi::ShaderType::Compute;
				default: return nvrhi::ShaderType::None;
			}
		}

		inline std::string ShaderStageSuffix(ShaderStage aStage)
		{
			switch (aStage)
			{
				case Rendering::ShaderStage::Vertex:	return "VS";
				case Rendering::ShaderStage::Geometry:	return "GS";
				case Rendering::ShaderStage::Pixel:		return "PS";
				case Rendering::ShaderStage::Compute:	return "CS";
				default: return "UNKOWN";
			}
		}
	}

	Shader::Shader() = default;

	Shader::Shader(const std::string& aDebugName, const std::map<ShaderStage, std::vector<uint8_t>>& aBinaries)
	{
		for (const auto& [stage, binary] : aBinaries)
		{
			nvrhi::ShaderDesc desc;
			desc.setShaderType(Utils::NVRHIShaderStage(stage));
			desc.setDebugName(std::format("{}_{}", aDebugName, Utils::ShaderStageSuffix(stage)));

			myShaderHandles[stage] = RenderContext::Get().DeviceManager->GetDeviceHandle()->createShader(desc, binary.data(), binary.size());
		}
	}

	Shader::~Shader() = default;
}
