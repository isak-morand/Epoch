#pragma once
#include "ShaderStage.h"
#include <nvrhi/nvrhi.h>
#include <map>

namespace Epoch::Rendering
{
	class Shader
	{
	public:
		Shader();
		Shader(const std::string& aDebugName, const std::map<ShaderStage, std::vector<uint8_t>>& aBinaries);
		~Shader();

		bool HasShader(ShaderStage aStage) { return myShaderHandles.find(aStage) != myShaderHandles.end(); }
		nvrhi::ShaderHandle GetShader(ShaderStage aStage) const { return myShaderHandles.at(aStage); }

	private:
		std::map<ShaderStage, nvrhi::ShaderHandle> myShaderHandles;
	};
}
