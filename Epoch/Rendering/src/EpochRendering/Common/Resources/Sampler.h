#pragma once
#include <CommonUtilities/Color.h>
#include <EpochDataTypes/TextureData.h>
#include <nvrhi/nvrhi.h>

namespace Epoch::Rendering
{
	struct SamplerSpecification
	{
		TextureFilter MinFilter = TextureFilter::Linear;
		TextureFilter MagFilter = TextureFilter::Linear;
		TextureFilter MipFilter = TextureFilter::Linear;
		TextureWrap AddressU = TextureWrap::Wrap;
		TextureWrap AddressV = TextureWrap::Wrap;
		TextureWrap AddressW = TextureWrap::Wrap;

		CU::Color BoarderColor = CU::Color::White;

		AnisotropyLevel AnisotropyLevel = AnisotropyLevel::None;

		void SetFilterMode(TextureFilter aMode) { MinFilter = MagFilter = MipFilter = aMode; }
		void SetWrapMode(TextureWrap aMode) { AddressU = AddressV = AddressW = aMode; }
	};

	class Sampler
	{
	public:
		Sampler() = delete;
		Sampler(SamplerSpecification aSpecification);
		~Sampler() = default;

		nvrhi::SamplerHandle GetHandle() const { return myHandler; }

	private:
		SamplerSpecification mySpecification;
		nvrhi::SamplerHandle myHandler;
	};
}
