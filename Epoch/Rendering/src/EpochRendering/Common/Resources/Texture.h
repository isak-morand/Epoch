#pragma once
#include "EpochDataTypes/TextureData.h"
#include <nvrhi/nvrhi.h>
#include "Image.h"
#include "Sampler.h"

namespace Epoch::Rendering
{
	struct TextureSpecification
	{
		ImageSpecification ImageSpec;
		SamplerSpecification SamplerSpec;
	};

	class Texture
	{
	public:
		Texture() = default;
		~Texture() = default;
	};

	class Texture2D : public Texture
	{
	public:
		Texture2D() = delete;
		Texture2D(const TextureSpecification& aSpecification);
		~Texture2D() = default;

		std::shared_ptr<Image2D> GetImage() const { return myImage; }
		std::shared_ptr<Sampler> GetSampler() const { return mySampler; }

	private:
		TextureSpecification mySpecification;

		std::shared_ptr<Image2D> myImage;
		std::shared_ptr<Sampler> mySampler;
	};
}
