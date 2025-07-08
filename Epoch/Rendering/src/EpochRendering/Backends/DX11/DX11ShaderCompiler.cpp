#include "epch.h"
#include "DX11ShaderCompiler.h"
#include <d3dcompiler.h>
#include <wrl/client.h>

using namespace Microsoft::WRL;

namespace Epoch::Rendering
{
	namespace Utils
	{
		inline const char* Target(ShaderStage aStage)
		{
			switch (aStage)
			{
				case ShaderStage::Vertex:	return "vs_5_0";
				case ShaderStage::Geometry:	return "gs_5_0";
				case ShaderStage::Pixel:	return "ps_5_0";
				case ShaderStage::Compute:	return "cs_5_0";
			}

			EPOCH_ASSERT(false, "Unknown shader stage!");
			return "";
		}
	}

	bool DX11ShaderCompiler::CompileBinary(ShaderStage aStage)
	{
		const std::string& source = myShaderSources[aStage];

		ComPtr<ID3DBlob> blob;
		ComPtr<ID3DBlob> errorBlob;
		HRESULT result = D3DCompile(
			source.c_str(),													//pSrcData
			source.length(),												//SrcDataSize
			myShaderSourcePath.string().c_str(),							//pSourceName
			NULL,															//*pDefines
			D3D_COMPILE_STANDARD_FILE_INCLUDE,								//*pInclude
			"main",															//pEntrypoint
			Utils::Target(aStage),													//pTarget
			D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3,	//Flags1
			NULL,															//Flags2
			&blob,															//**ppCode
			&errorBlob);													//**ppErrorMsgs

		if (FAILED(result))
		{
			LOG_ERROR("Failed to compile {} - {} shader!\nError: {}", myShaderSourcePath.stem().string(), ShaderTypeToString(aStage), (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
			if (blob)
			{
				blob->Release();
			}
			return false;
		}
		else
		{
			auto& binaries = myBinaries[aStage];
			const size_t size = blob->GetBufferSize();
			binaries.resize(size);
			std::memcpy(binaries.data(), blob->GetBufferPointer(), size);
			blob->Release();
		}

		return true;
	}
}
