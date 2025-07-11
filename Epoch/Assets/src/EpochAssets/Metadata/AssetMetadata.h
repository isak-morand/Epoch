#pragma once
#include <variant>
#include <filesystem>
#include "EpochAssets/AssetTypes.h"
#include "EpochAssets/AssetHandle.h"
#include <EpochDataTypes/TextureData.h>

namespace Epoch::Assets
{
	struct TextureImportSettings
	{
		TextureFilter FilterMode = TextureFilter::Linear;
		TextureWrap WrapMode = TextureWrap::Wrap;

		bool GenerateMips = false;//true;
		AnisotropyLevel AnisotropyLevel = AnisotropyLevel::None;
	};

	struct ModelImportSettings
	{
		// If hierarchy is flattened, Hierarchy will have one node.
		// If hierarchy is not flattened, Hierarchy mirrors the original structure.

		bool FlattenHierarchy = false;
	};

	using ImportSettingsVariant = std::variant<std::monostate, TextureImportSettings, ModelImportSettings>;

	struct AssetMetadata
	{
		AssetHandle Handle = 0;
		AssetType Type = AssetType::None;
		bool IsMemoryAsset = false;

		std::filesystem::path FilePath = "";

		ImportSettingsVariant ImportSettings;

		bool IsValid() const { return Type != AssetType::None; }

		template<typename T>
		const T& GetImportSettings() const
		{
			return std::get<T>(ImportSettings);
		}
	};
}
