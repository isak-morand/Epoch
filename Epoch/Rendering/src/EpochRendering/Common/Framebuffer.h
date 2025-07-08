#pragma once
#include <map>
#include <nvrhi/nvrhi.h>
#include <EpochCore/Assert.h>
#include <EpochDataTypes/TextureData.h>

namespace Epoch::Rendering
{
	class Image2D;
	class Framebuffer;

	struct FramebufferTextureSpecification
	{
		ImageFormat Format;
		std::string DebugName = "";

		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(ImageFormat aFormat, std::string_view aDebugName = "Attachment") : Format(aFormat), DebugName(aDebugName) {}
	};

	struct FramebufferAttachmentSpecification
	{
		std::vector<FramebufferTextureSpecification> Attachments;

		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(const std::initializer_list<FramebufferTextureSpecification>& aAttachments) : Attachments(aAttachments) {}
	};

	struct FramebufferSpecification
	{
		uint32_t Width = 0;
		uint32_t Height = 0;

		bool Resize = true;

		FramebufferAttachmentSpecification Attachments;

		//Specify existing images to attach instead of creating new images. Attachment index -> Image
		std::map<uint32_t, std::shared_ptr<Image2D>> ExistingImages;

		//Will just create a new render pass with an existing framebuffer
		std::shared_ptr<Framebuffer> ExistingFramebuffer;

		//TODO: Make this work
		bool SwapChainTarget = false;

		std::string DebugName = "Framebuffer";
	};

	class Framebuffer
	{
	public:
		Framebuffer() = delete;
		Framebuffer(const FramebufferSpecification& aSpecification);
		~Framebuffer();

		nvrhi::FramebufferHandle GetHandle() const;

		std::shared_ptr<Image2D> GetAttachment(uint32_t aAttachmentIndex = 0) const { return myAttachmentImages.at(aAttachmentIndex); }
		std::shared_ptr<Image2D> GetAttachment(const std::string& aName) const { return myAttachmentImages.at(myAttachmentNameMap.at(aName)); }
		std::shared_ptr<Image2D> GetDepthAttachment() const { return myDepthAttachmentImage; }
		size_t GetAttachmentCount() const { return /*mySpecification.SwapChainTarget ? 1 : */myAttachmentImages.size(); }
		bool HasDepthAttachment() const { return (bool)myDepthAttachmentImage; }

		uint32_t GetWidth() const { return myWidth; }
		uint32_t GetHeight() const { return myHeight; }

		void Resize(uint32_t aWidth, uint32_t aHeight);

	private:
		FramebufferSpecification mySpecification;
		nvrhi::FramebufferHandle myHandle;

		uint32_t myWidth = 0, myHeight = 0;

		std::vector<std::shared_ptr<Image2D>> myAttachmentImages;
		std::shared_ptr<Image2D> myDepthAttachmentImage;

		std::unordered_map<std::string, uint32_t> myAttachmentNameMap;
	};
}
