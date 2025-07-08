#include "epch.h"
#include "Framebuffer.h"
#include "RenderContext.h"
#include "Resources/Image.h"

namespace Epoch::Rendering
{
	Framebuffer::Framebuffer(const FramebufferSpecification& aSpecification) : mySpecification(aSpecification)
	{
		if (mySpecification.SwapChainTarget)
		{
			myWidth = RenderContext::Get().SwapChain->GetWidth();
			myHeight = RenderContext::Get().SwapChain->GetHeight();
			return;
		}
		else
		{
			EPOCH_ENSURE(mySpecification.Width > 0 && mySpecification.Height > 0,
						 "Failed to create framebuffer ('{}')! A Width and/or height of zero isn't allowed!", mySpecification.DebugName);

			myWidth = mySpecification.Width;
			myHeight = mySpecification.Height;
		}

		if (mySpecification.ExistingFramebuffer)
		{
			for (size_t i = 0; i < mySpecification.ExistingFramebuffer->GetAttachmentCount(); ++i)
			{
				myAttachmentImages.emplace_back(mySpecification.ExistingFramebuffer->GetAttachment((uint32_t)i));
			}

			if (mySpecification.ExistingFramebuffer->HasDepthAttachment())
			{
				myDepthAttachmentImage = mySpecification.ExistingFramebuffer->GetDepthAttachment();
			}

			myAttachmentNameMap = mySpecification.ExistingFramebuffer->myAttachmentNameMap;
		}
		else
		{
			int attachmentIndex = 0;
			bool depthAttachmentCreated = false;

			for (auto& attachmentSpec : mySpecification.Attachments.Attachments)
			{
				std::string attachmentName = attachmentSpec.DebugName;
				EPOCH_ASSERT(!attachmentName.empty(), "A frame buffers attachments name can't be empty!");

				if (DataTypes::IsDepthFormat(attachmentSpec.Format))
				{
					if (depthAttachmentCreated)
					{
						LOG_ERROR("A framebuffer can't have more than one depth attachment! '{}'", mySpecification.DebugName);
						continue;
					}

					if (auto it = mySpecification.ExistingImages.find(attachmentIndex); it != mySpecification.ExistingImages.end())
					{
						myDepthAttachmentImage = it->second;
					}
					else
					{
						ImageSpecification spec;
						spec.Format = attachmentSpec.Format;
						spec.Usage = ImageUsage::RenderTarget;
						spec.Width = myWidth;
						spec.Height = myHeight;
						spec.DebugName = std::format("{} - {}", mySpecification.DebugName, attachmentName);

						myDepthAttachmentImage = std::make_shared<Image2D>(spec);
					}
				}
				else
				{
					if (auto it = mySpecification.ExistingImages.find(attachmentIndex); it != mySpecification.ExistingImages.end())
					{
						myAttachmentImages.push_back(it->second);
					}
					else
					{
						ImageSpecification spec;
						spec.Format = attachmentSpec.Format;
						spec.Usage = ImageUsage::RenderTarget;
						spec.Width = myWidth;
						spec.Height = myHeight;
						spec.DebugName = std::format("{} - {}", mySpecification.DebugName, attachmentName);

						myAttachmentImages.emplace_back(std::make_shared<Image2D>(spec));
					}

					myAttachmentNameMap[attachmentName] = (uint32_t)myAttachmentImages.size();
				}

				attachmentIndex++;
			}
		}

		nvrhi::FramebufferDesc framebufferDesc;

		if (myDepthAttachmentImage)
		{
			framebufferDesc.setDepthAttachment(myDepthAttachmentImage->GetHandle());
		}

		for (const auto& attachment : myAttachmentImages)
		{
			framebufferDesc.addColorAttachment(attachment->GetHandle());
		}

		myHandle = RenderContext::Get().DeviceManager->GetDeviceHandle()->createFramebuffer(framebufferDesc);
	}

	nvrhi::FramebufferHandle Framebuffer::GetHandle() const
	{
		if (mySpecification.SwapChainTarget)
		{
			return RenderContext::Get().SwapChain->GetFrameBuffer();
		}
		return myHandle;
	}

	void Framebuffer::Resize(uint32_t aWidth, uint32_t aHeight)
	{
		if (!mySpecification.Resize || (myWidth == aWidth && myHeight == aHeight))
		{
			return;
		}

		myWidth = aWidth;
		myHeight = aHeight;

		if (mySpecification.SwapChainTarget || mySpecification.ExistingFramebuffer)
		{
			return;
		}

		EPOCH_ASSERT(false, "Framebuffer::Resize not implemented!");
	}

	Framebuffer::~Framebuffer() = default;
}
