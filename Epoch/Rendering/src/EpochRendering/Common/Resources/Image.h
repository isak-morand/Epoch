#pragma once
#include <nvrhi/nvrhi.h>
#include <EpochCore/Buffer.h>
#include <EpochDataTypes/TextureData.h>

namespace Epoch::Rendering
{
	namespace Utils
	{
		inline nvrhi::Format GetNVRHIFormat(ImageFormat aFormat)
		{
			switch (aFormat)
			{
				case ImageFormat::RGBA:				return nvrhi::Format::RGBA8_UNORM;
				case ImageFormat::RGBA32F:			return nvrhi::Format::RGBA32_FLOAT;
				case ImageFormat::R11G11B10F:		return nvrhi::Format::R11G11B10_FLOAT;
				case ImageFormat::RG16F:			return nvrhi::Format::RG16_FLOAT;
				case ImageFormat::RG16UNORM:		return nvrhi::Format::RG16_UNORM;
				case ImageFormat::DEPTH32:			return nvrhi::Format::D32;
			}
			EPOCH_ASSERT(false, "Unknown texture format!");
			return nvrhi::Format::UNKNOWN;
		}
	}

	enum class ImageUsage : uint8_t
	{
		None			= 0,
		Resource		= 1 << 0,
		RenderTarget	= 1 << 1
	};

	struct ImageSpecification
	{
		ImageFormat Format = ImageFormat::RGBA;

		uint32_t Width = 1;
		uint32_t Height = 1;

		ImageUsage Usage = ImageUsage::Resource;

		bool GenerateMipmaps = false;

		Core::Buffer InitialData;

		std::string DebugName;
	};

	class Image
	{
	public:
		virtual ~Image();

		nvrhi::TextureHandle GetHandle() const { return myHandle; }

		ImageFormat GetFormat() const { return mySpecification.Format; }

	protected:
		Image(const ImageSpecification& aSpecification);

	protected:
		ImageSpecification mySpecification;
		nvrhi::TextureHandle myHandle;
	};

	class Image2D : public Image
	{
	public:
		Image2D() = delete;
		Image2D(const ImageSpecification& aSpecification);
		~Image2D() override;

	private:

	};
}
