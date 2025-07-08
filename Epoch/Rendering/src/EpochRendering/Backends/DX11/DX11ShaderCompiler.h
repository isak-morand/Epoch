#pragma once
#include "EpochRendering/Common/Shaders/ShaderCompiler.h"

namespace Epoch::Rendering
{
	class DX11ShaderCompiler : public ShaderCompiler
	{
	public:
		DX11ShaderCompiler() = default;
		~DX11ShaderCompiler() override = default;

	protected:
		bool CompileBinary(ShaderStage aStage) override;
	};
}
