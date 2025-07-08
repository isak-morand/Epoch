#pragma once
#include <filesystem>
#include <map>
#include <EpochDataTypes/ShaderProperty.h>
#include "ShaderStage.h"

namespace Epoch::Rendering
{
	class ShaderCompiler
	{
	public:
		virtual ~ShaderCompiler();

		static std::unique_ptr<ShaderCompiler> Create();

		bool Compile(const std::filesystem::path& aShaderSourcePath);

		const std::map<ShaderStage, std::vector<uint8_t>>& GetBinaries() const { return myBinaries; }
		const std::vector<std::unique_ptr<DataTypes::ShaderProperty>>& GetProperties() const { return myShaderProperties; }

	protected:
		bool PreProcess();

		void ParseProperties(const std::string& aShaderSource);
		void AlignBufferParameters();
		std::string GenerateDeclarations();
		std::string ExtractAndInjectShader(const std::string& aShaderCode, const std::string& aStage, const std::string& aInjections);

		bool CompileBinaries();
		virtual bool CompileBinary(ShaderStage aStage) = 0;

	protected:
		std::filesystem::path myShaderSourcePath;

		std::vector<std::unique_ptr<DataTypes::ShaderProperty>> myShaderProperties;

		std::map<ShaderStage, std::string> myShaderSources;
		std::map<ShaderStage, std::vector<uint8_t>> myBinaries;
	};
}
