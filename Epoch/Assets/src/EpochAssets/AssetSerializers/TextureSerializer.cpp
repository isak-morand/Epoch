#include "TextureSerializer.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#include <EpochCore/Log.h>
#include <EpochCore/FileSystem.h>
#include "EpochAssets/AssetManager.h"
#include "EpochAssets/Assets/TextureAsset.h"

namespace Epoch::Assets
{
	bool TextureSerializer::TryLoadData(const AssetMetadata& aMetadata, std::shared_ptr<Asset>& outAsset) const
	{
		const TextureImportSettings& importSettings = aMetadata.GetImportSettings<TextureImportSettings>();

		DataTypes::TextureData textureData;
		if (!FillTextureData(AssetManager::GetEditorAssetManager()->GetFileSystemPath(aMetadata.FilePath), textureData, importSettings))
		{
			return false;
		}

		auto textureAsset = std::make_shared<TextureAsset>(aMetadata.Handle);
		textureAsset->SetData(std::move(textureData));
		outAsset = textureAsset;

		return true;
	}

	bool TextureSerializer::FillTextureData(const std::filesystem::path& aFilePath, DataTypes::TextureData& outTextureData, const TextureImportSettings& aImportSettings) const
	{
		Core::Buffer fileData = Core::FileSystem::ReadFile(aFilePath);

		int width = 0, height = 0, originalChannels = 0, channels = 0;

		if (!stbi_info_from_memory(
			static_cast<const stbi_uc*>(fileData.data),
			static_cast<int>(fileData.size),
			&width, &height, &originalChannels))
		{
			LOG_ERROR("Couldn't process image header for texture '%s'", aFilePath.string());
			return false;
		}

		bool isHDR = stbi_is_hdr_from_memory(
			static_cast<const stbi_uc*>(fileData.data),
			static_cast<int>(fileData.size));

		if (originalChannels == 3)
		{
			channels = 4;
		}
		else
		{
			channels = originalChannels;
		}

		unsigned char* bitmap;
		int bytesPerPixel = channels * (isHDR ? 4 : 1);

		if (isHDR)
		{
			float* floatmap = stbi_loadf_from_memory(
				static_cast<const stbi_uc*>(fileData.data),
				static_cast<int>(fileData.size),
				&width, &height, &originalChannels, channels);

			bitmap = reinterpret_cast<unsigned char*>(floatmap);
		}
		else
		{
			bitmap = stbi_load_from_memory(
				static_cast<const stbi_uc*>(fileData.data),
				static_cast<int>(fileData.size),
				&width, &height, &originalChannels, channels);
		}

		fileData.Release();

		if (!bitmap)
		{
			LOG_ERROR("Couldn't load texture '%s'", aFilePath.string());
			return false;
		}

		switch (channels)
		{
			//case 1:
			//{
			//	break;
			//}
			//case 2:
			//{
			//	break;
			//}
			case 4:
			{
				outTextureData.Format = isHDR ? ImageFormat::RGBA32F : ImageFormat::RGBA;
				break;
			}
			default:
			{
				LOG_ERROR("Unsupported number of channels (%d) for texture '%s'", aFilePath.string());
				stbi_image_free(bitmap);
				return false;
				break;
			}
		}

		outTextureData.Width = (uint32_t)width;
		outTextureData.Height = (uint32_t)height;
		outTextureData.Data = Core::Buffer::Copy({ (void*)bitmap, (uint64_t)(bytesPerPixel * width * height) });
		stbi_image_free(bitmap);

		outTextureData.FilterMode = aImportSettings.FilterMode;
		outTextureData.WrapMode = aImportSettings.WrapMode;
		outTextureData.GenerateMips = aImportSettings.GenerateMips;

		return true;
	}
}
