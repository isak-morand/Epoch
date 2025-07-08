#pragma once
#include <EpochCore/Buffer.h>

namespace Epoch
{
	enum class TextureFilter : uint8_t
	{
		None = 0,
		Linear,
		Nearest
	};

	enum class TextureWrap : uint8_t
	{
		None = 0,
		Clamp,
		Wrap,
		Border
	};

	enum class AnisotropyLevel : uint8_t
	{
		None = 0,
		_2,
		_4,
		_8,
		_16
	};

	namespace Utils
	{
		inline TextureFilter FilterModeFromString(std::string_view aFilterMode)
		{
			if (aFilterMode == "None")		return TextureFilter::None;
			if (aFilterMode == "Linear")	return TextureFilter::Linear;
			if (aFilterMode == "Nearest")	return TextureFilter::Nearest;

			EPOCH_ASSERT(false, "Unknown Texture Filter Mode");
			return TextureFilter::None;
		}

		inline const char* FilterModeToString(TextureFilter aFilterMode)
		{
			switch (aFilterMode)
			{
				case TextureFilter::Linear:		return "Linear";
				case TextureFilter::Nearest:	return "Nearest";
			}

			EPOCH_ASSERT(false, "Unknown Texture Filter Mode");
			return "None";
		}

		inline TextureWrap WrapModeFromString(std::string_view aWrapMode)
		{
			if (aWrapMode == "None")		return TextureWrap::None;
			if (aWrapMode == "Wrap")		return TextureWrap::Wrap;
			if (aWrapMode == "Clamp")		return TextureWrap::Clamp;
			if (aWrapMode == "Border")		return TextureWrap::Border;

			EPOCH_ASSERT(false, "Unknown Texture Wrap Mode");
			return TextureWrap::None;
		}

		inline const char* WrapModeToString(TextureWrap aWrapMode)
		{
			switch (aWrapMode)
			{
				case TextureWrap::Wrap:		return "Wrap";
				case TextureWrap::Clamp:	return "Clamp";
				case TextureWrap::Border:	return "Border";
			}

			EPOCH_ASSERT(false, "Unknown Texture Wrap Mode");
			return "None";
		}
	}

	enum class ImageFormat
	{
		None = 0,

		RGBA,
		RGBA32F,
		R11G11B10F,
		RG16F,
		RG16UNORM,

		DEPTH32
	};
}

namespace Epoch::DataTypes
{
	inline bool IsDepthFormat(ImageFormat aFormat)
	{
		if (aFormat == ImageFormat::DEPTH32)
		{
			return true;
		}
		return false;
	}

	struct TextureData
	{
		Core::Buffer Data;

		uint32_t Width = 0;
		uint32_t Height = 0;

		ImageFormat Format = ImageFormat::None;
		TextureFilter FilterMode = TextureFilter::Linear;
		TextureWrap WrapMode = TextureWrap::Wrap;

		bool GenerateMips = true;
		AnisotropyLevel AnisotropyLevel = AnisotropyLevel::None;

		bool IsValid() const { return (bool)(Data.data) && Format != ImageFormat::None; }
	};
}
