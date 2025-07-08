#include "epch.h"
#include "ShaderCompiler.h"

#if defined(USE_DX11)
#include <EpochRendering/Backends/DX11/DX11ShaderCompiler.h>
#elif defined(USE_DX12)
#include "EpochRendering/Backends/DX12/DX12ShaderCompiler.h"
#elif defined(USE_VULKAN)
#include "EpochRendering/Backends/Vulkan/VKDeviceManager.h"
#endif

#include <CommonUtilities/Timer.h>
#include <regex>

namespace Epoch::Rendering
{
	ShaderCompiler::~ShaderCompiler() = default;

	std::unique_ptr<ShaderCompiler> ShaderCompiler::Create()
	{
#if defined(USE_DX11)
		return std::make_unique<DX11ShaderCompiler>();
#elif defined(USE_DX12)
		return std::make_unique<DX12ShaderCompiler>();
#elif defined(USE_VULKAN)
		return std::make_unique<VKDeviceManager>();
#else
		return nullptr;
#endif
	}

	bool ShaderCompiler::Compile(const std::filesystem::path& aShaderSourcePath)
	{
		myShaderSourcePath = aShaderSourcePath;
		myShaderProperties.clear();
		myShaderSources.clear();
		myBinaries.clear();

		CU::Timer timer;

		if (!PreProcess())
		{
			LOG_ERROR("Failed to pre process shader '{}'!", myShaderSourcePath.string());
			return false;
		}

		if (!CompileBinaries())
		{
			return false;
		}

		LOG_INFO("Successfully compiled {} shaders from '{}' Time: {}ms", myBinaries.size(), aShaderSourcePath.string(), timer.ElapsedMillis());
		return true;
	}

	bool ShaderCompiler::PreProcess()
	{
		std::string source = CU::ReadFileAndSkipBOM(myShaderSourcePath);
		if (source.empty())
		{
			LOG_ERROR("Failed to read file '{}'!", myShaderSourcePath.string());
			return false;
		}

		ParseProperties(source);

		std::string pixelInjections = GenerateDeclarations();

		myShaderSources[ShaderStage::Vertex] = ExtractAndInjectShader(source, "VertexShader", "");
		myShaderSources[ShaderStage::Pixel] = ExtractAndInjectShader(source, "PixelShader", pixelInjections);

		//LOG_DEBUG("\n\nVertex Shader:\n{}\n\n", myShaderSources[ShaderStage::Vertex]);
		//LOG_DEBUG("\n\nPixel Shader:\n{}\n\n", myShaderSources[ShaderStage::Pixel]);

		return true;
	}

	void ShaderCompiler::ParseProperties(const std::string& aShaderSource)
	{
		std::smatch match;
		std::regex propertiesBlock(R"(Properties\s*\{([^}]*)\})", std::regex::optimize);
		//std::regex propLine(R"((\w+)\s*\(\s*"([^"]+)\"\s*,\s*(\w+)\s*\)\s*=\s*(\([^)]+\)|"[^"]+"))");
		std::regex propLine(R"((\w+)\s*\(\s*"([^"]+)\"\s*,\s*(\w+)(?:\(([^,]+),\s*([^)]+)\))?\s*\)\s*=\s*([^{}\r\n]+))");

		if (std::regex_search(aShaderSource, match, propertiesBlock))
		{
			using PropertyType = DataTypes::ShaderPropertyType;

			std::string body = match[1].str();
			std::sregex_iterator it(body.begin(), body.end(), propLine), end;
			while (it != end)
			{
				std::string name = (*it)[1];
				std::string displayName = (*it)[2];
				std::string sType = (*it)[3];
				std::string defaultValue = (*it)[6];

				PropertyType type = DataTypes::StringToShaderPropertyType(sType);
				if (type == PropertyType::Unkown)
				{
					LOG_ERROR("Unknown shader property type!");
				}
				else
				{
					std::unique_ptr<DataTypes::ShaderProperty> property = DataTypes::CreateProperty(type);

					property->Name = name;
					property->DisplayName = displayName;
					property->Type = type;

					if (property->ToDefaultValue(defaultValue))
					{
						if (type == DataTypes::ShaderPropertyType::Range && ((*it)[4].matched && (*it)[5].matched))
						{
							DataTypes::RangeProperty* rangeProperty = (DataTypes::RangeProperty*)property.get();

							rangeProperty->ToMinValue((*it)[4]);
							rangeProperty->ToMaxValue((*it)[5]);
						}

						myShaderProperties.push_back(std::move(property));
					}
				}
				
				++it;
			}
		}

		AlignBufferParameters();
	}

	inline uint32_t AlignTo(uint32_t aValue, uint32_t aAlignment)
	{
		return (aValue + aAlignment - 1) & ~(aAlignment - 1);
	}

	void ShaderCompiler::AlignBufferParameters()
	{
		using PropertyType = DataTypes::ShaderPropertyType;

		uint32_t bufferOffset = 0;
		for (auto& prop : myShaderProperties)
		{
			PropertyType type = prop->Type;

			if (type == PropertyType::Tex2D) continue;

			uint32_t size = DataTypes::GetPropertyByteSize(type);

			bufferOffset = AlignTo(bufferOffset, size);
			prop->OffsetInBuffer = bufferOffset;
			prop->SizeInBuffer = size;
			bufferOffset += size;
		}
	}

	std::string ShaderCompiler::GenerateDeclarations()
	{
		using PropertyType = DataTypes::ShaderPropertyType;

		std::ostringstream oss;

		// CBuffer for constants
		bool hasCBuffer = false;
		for (const auto& prop : myShaderProperties)
		{
			PropertyType type = prop->Type;

			if (type == PropertyType::Color || type == PropertyType::Float || type == PropertyType::Vector)
			{
				if (!hasCBuffer)
				{
					oss << "cbuffer MaterialProperties : register(b0)\n{\n";
					hasCBuffer = true;
				}

				if (type == PropertyType::Color || type == PropertyType::Vector)
				{
					oss << "    float4 " << prop->Name << ";\n";
				}
				else if (type == PropertyType::Float || type == PropertyType::Range)
				{
					oss << "    float " << prop->Name << ";\n";
				}
			}
		}
		if (hasCBuffer) oss << "};\n\n";

		// Texture and Sampler declarations
		int reg = 0;
		for (const auto& prop : myShaderProperties)
		{
			if (prop->Type == PropertyType::Tex2D)
			{
				oss << "Texture2D " << prop->Name << " : register(t" << reg << ");\n";
				oss << "SamplerState " << prop->Name << "Sampler : register(s" << reg << ");\n";
				++reg;
			}
		}

		return oss.str();
	}

	std::string ShaderCompiler::ExtractAndInjectShader(const std::string& aShaderCode, const std::string& aStage, const std::string& aInjections)
	{
		size_t blockStart = aShaderCode.find(aStage);
		if (blockStart == std::string::npos) return "";

		size_t braceOpen = aShaderCode.find('{', blockStart);
		if (braceOpen == std::string::npos) return "";

		int braceCount = 1;
		size_t i = braceOpen + 1;
		while (i < aShaderCode.size() && braceCount > 0)
		{
			if (aShaderCode[i] == '{') braceCount++;
			else if (aShaderCode[i] == '}') braceCount--;
			i++;
		}

		if (braceCount != 0) return ""; // unmatched braces

		auto body = aShaderCode.substr(braceOpen + 1, i - braceOpen - 2);

		std::ostringstream finalShader;
		finalShader << aInjections << "\n" << body;
		return finalShader.str();
	}

	bool ShaderCompiler::CompileBinaries()
	{
		for (auto [stage, source] : myShaderSources)
		{
			if (!CompileBinary(stage))
			{
				return false;
			}
		}

		return true;
	}

}