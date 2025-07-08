#pragma once
#include "EpochRendering/Common/Shader/ShaderCompiler.h"

namespace Epoch::Rendering
{
	class DX12ShaderCompiler : public ShaderCompiler
	{
	public:
		DX12ShaderCompiler() = default;
		~DX12ShaderCompiler() override = default;

	protected:
		bool CompileBinary(ShaderStage aStage) override;
	};
}
