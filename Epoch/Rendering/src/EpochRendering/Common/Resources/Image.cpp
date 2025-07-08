#include "epch.h"
#include "Image.h"
#include "EpochRendering/Common/RenderContext.h"

namespace Epoch::Rendering
{
	namespace Utils
	{
		inline uint32_t GetImageFormatBPP(ImageFormat aFormat)
		{
			switch (aFormat)
			{
				case ImageFormat::RGBA:			return 4;
				case ImageFormat::RGBA32F:		return 4 * sizeof(uint32_t);
				case ImageFormat::R11G11B10F:	return 4;
				case ImageFormat::RG16F:			return 2 * sizeof(uint16_t);
				case ImageFormat::RG16UNORM:		return 2 * sizeof(uint16_t);
				case ImageFormat::DEPTH32:		return sizeof(uint32_t);
			}
			EPOCH_ASSERT(false, "Unkown image format");
			return 0;
		}

		inline uint32_t GetImageMemoryRowPitch(ImageFormat aFormat, uint32_t aWidth)
		{
			return aWidth * GetImageFormatBPP(aFormat);
		}

		inline uint32_t CalculateMipCount(uint32_t aWidth, uint32_t aHeight)
		{
			uint32_t size = std::min(aWidth, aHeight);
			return (uint32_t)std::log2(size) + 1;
		}
	}

	Image::Image(const ImageSpecification& aSpecification) : mySpecification(aSpecification) {}

	Image::~Image()
	{
		if (mySpecification.InitialData)
		{
			mySpecification.InitialData.Release();
		}
	}

	Image2D::Image2D(const ImageSpecification& aSpecification) : Image(aSpecification)
	{
		EPOCH_ASSERT(mySpecification.Width > 0u && mySpecification.Height > 0u, "Images can't be created with a width and/or height of zero!");

		nvrhi::TextureDesc desc;

		desc.width = mySpecification.Width;
		desc.height = mySpecification.Height;
		desc.format = Utils::GetNVRHIFormat(mySpecification.Format);

		desc.isShaderResource = (uint8_t)mySpecification.Usage & (uint8_t)ImageUsage::Resource;
		desc.isRenderTarget = (uint8_t)mySpecification.Usage & (uint8_t)ImageUsage::RenderTarget;

		desc.initialState = desc.isRenderTarget ? nvrhi::ResourceStates::RenderTarget : desc.isShaderResource ? nvrhi::ResourceStates::ShaderResource : nvrhi::ResourceStates::Unknown;

		desc.keepInitialState = true;

		if (mySpecification.GenerateMipmaps)
		{
			if (((mySpecification.Width & (mySpecification.Width - 1)) == 0) && ((mySpecification.Height & (mySpecification.Height - 1)) == 0))
			{
				//desc.mipLevels = 0; // generate full chain
				desc.mipLevels = Utils::CalculateMipCount(mySpecification.Width, mySpecification.Height);
			}
			else
			{
				LOG_WARNING("Image isn't power of two. No mip maps will be generated!");
			}
		}

		desc.debugName = mySpecification.DebugName;

		auto device = RenderContext::Get().DeviceManager->GetDeviceHandle();
		myHandle = device->createTexture(desc);

		if (mySpecification.InitialData)
		{
			nvrhi::CommandListHandle commandList = device->createCommandList();
			commandList->open();
			commandList->writeTexture(myHandle, 0, 0, mySpecification.InitialData.data, Utils::GetImageMemoryRowPitch(mySpecification.Format, mySpecification.Width));
			commandList->close();
			device->executeCommandList(commandList);
		}
	}

	Image2D::~Image2D() = default;
}
