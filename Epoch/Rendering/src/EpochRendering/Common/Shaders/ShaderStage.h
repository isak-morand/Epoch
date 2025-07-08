#pragma once
#include <EpochCore/Assert.h>

namespace Epoch::Rendering
{
	enum class ShaderStage : uint8_t
	{
		None, Vertex, Geometry, Pixel, Compute
	};

	inline static ShaderStage ShaderStageFromString(const std::string_view aType)
	{
		if (aType == "Vertex")
		{
			return ShaderStage::Vertex;
		}

		if (aType == "Geometry")
		{
			return ShaderStage::Geometry;
		}

		if (aType == "Pixel")
		{
			return ShaderStage::Pixel;
		}

		if (aType == "Compute")
		{
			return ShaderStage::Compute;
		}

		EPOCH_ASSERT(false, "Unknown shader type!");
		return ShaderStage::None;
	}

	inline static std::string ShaderTypeToString(ShaderStage aType)
	{
		if (aType == ShaderStage::Vertex)
		{
			return "Vertex";
		}

		if (aType == ShaderStage::Geometry)
		{
			return "Geometry";
		}

		if (aType == ShaderStage::Pixel)
		{
			return "Pixel";
		}

		if (aType == ShaderStage::Compute)
		{
			return "Compute";
		}

		EPOCH_ASSERT(false, "Unknown shader type!");
		return "Unknown";
	}
}
