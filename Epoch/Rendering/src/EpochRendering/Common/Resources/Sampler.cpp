#include "epch.h"
#include "Sampler.h"
#include "EpochRendering/Common/RenderContext.h"

namespace Epoch::Rendering
{
	namespace Utils
	{
		inline nvrhi::SamplerAddressMode NVRHISamplerWrap(TextureWrap aWrap)
		{
			switch (aWrap)
			{
				case TextureWrap::Clamp:		return nvrhi::SamplerAddressMode::Clamp;
				case TextureWrap::Wrap:			return nvrhi::SamplerAddressMode::Repeat;
				case TextureWrap::Border:		return nvrhi::SamplerAddressMode::Border;
			}
			EPOCH_ASSERT(false, "Unknown wrap mode");
			return (nvrhi::SamplerAddressMode)0;
		}

		inline bool NVRHISamplerFilter(TextureFilter aFilter)
		{
			switch (aFilter)
			{
				case TextureFilter::Linear:		return true;
				case TextureFilter::Nearest:	return false;
			}
			EPOCH_ASSERT(false, "Unknown filter");
			return false;
		}

		inline float NVRHIAnisotropyLevel(AnisotropyLevel aAnisotropyLevel)
		{
			switch (aAnisotropyLevel)
			{
			case AnisotropyLevel::None:		return 1.f;
			case AnisotropyLevel::_2:		return 2.f;
			case AnisotropyLevel::_4:		return 4.f;
			case AnisotropyLevel::_8:		return 8.f;
			case AnisotropyLevel::_16:		return 16.f;
			}
			EPOCH_ASSERT(false, "Unknown anisotropy level");
			return 1.f;
		}
	}

	Sampler::Sampler(SamplerSpecification aSpecification) : mySpecification(aSpecification)
	{
		nvrhi::SamplerDesc samplerDesc;

		samplerDesc.minFilter = Utils::NVRHISamplerFilter(mySpecification.MinFilter);
		samplerDesc.magFilter = Utils::NVRHISamplerFilter(mySpecification.MagFilter);
		samplerDesc.mipFilter = Utils::NVRHISamplerFilter(mySpecification.MipFilter);

		samplerDesc.addressU = Utils::NVRHISamplerWrap(mySpecification.AddressU);
		samplerDesc.addressV = Utils::NVRHISamplerWrap(mySpecification.AddressV);
		samplerDesc.addressW = Utils::NVRHISamplerWrap(mySpecification.AddressW);

		samplerDesc.borderColor =
		{ mySpecification.BoarderColor.r, mySpecification.BoarderColor.g, mySpecification.BoarderColor.b, mySpecification.BoarderColor.a };
		
		samplerDesc.maxAnisotropy = Utils::NVRHIAnisotropyLevel(mySpecification.AnisotropyLevel);

		myHandler = RenderContext::Get().DeviceManager->GetDeviceHandle()->createSampler(samplerDesc);
	}
}
