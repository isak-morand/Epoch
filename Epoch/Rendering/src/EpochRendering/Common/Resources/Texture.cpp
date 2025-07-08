#include "epch.h"
#include "Texture.h"

namespace Epoch::Rendering
{
	Texture2D::Texture2D(const TextureSpecification& aSpecification) : mySpecification(aSpecification)
	{
		myImage = std::make_shared<Image2D>(mySpecification.ImageSpec);
		mySampler = std::make_shared<Sampler>(mySpecification.SamplerSpec);
	}
}
