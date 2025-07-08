#include "AssetMetadataSerializer.h"
#include <fstream>
#include <EpochSerialization/YAMLHelpers.h>
#include <EpochDataTypes/TextureData.h>
#include "EpochAssets/AssetManager.h"

namespace Epoch::Assets
{
	void ImportSettingsFactory::Register(AssetType aType, ImportSettingsSerializerFn aSerializer, ImportSettingsDeserializerFn aDeserializer, ImportSettingsCreatorFn aCreator)
	{
		staticSerializers[aType] = std::move(aSerializer);
		staticDeserializers[aType] = std::move(aDeserializer);
		staticCreators[aType] = std::move(aCreator);
	}

	void ImportSettingsFactory::Serialize(AssetType aType, const ImportSettingsVariant& aSettings, YAML::Emitter& out)
	{
		if (auto it = staticSerializers.find(aType); it != staticSerializers.end())
		{
			return it->second(aSettings, out);
		}
	}

	ImportSettingsVariant ImportSettingsFactory::Deserialize(AssetType aType, const YAML::Node& aNode)
	{
		if (auto it = staticDeserializers.find(aType); it != staticDeserializers.end())
		{
			return it->second(aNode);
		}
		return std::monostate{};
	}

	ImportSettingsVariant ImportSettingsFactory::CreateDefault(AssetType aType)
	{
		if (auto it = staticCreators.find(aType); it != staticCreators.end())
		{
			return it->second();
		}
		return std::monostate{};
	}

	void AssetMetadataSerializer::Init()
	{
		ImportSettingsFactory::Register
		(
			AssetType::Texture,
			[](const ImportSettingsVariant& variant, YAML::Emitter& out)
			{
				const auto& settings = std::get<TextureImportSettings>(variant);
				out << YAML::Key << "Filter" << YAML::Value << Epoch::Utils::FilterModeToString(settings.FilterMode);
				out << YAML::Key << "Wrap" << YAML::Value << Epoch::Utils::WrapModeToString(settings.WrapMode);
				out << YAML::Key << "AnisotropyLevel" << YAML::Value << (uint8_t)settings.AnisotropyLevel;
			},
			[](const YAML::Node& aNode) -> ImportSettingsVariant
			{
				TextureImportSettings settings;
				settings.FilterMode = Epoch::Utils::FilterModeFromString(aNode["Filter"].as<std::string>(Epoch::Utils::FilterModeToString(TextureFilter::Linear)));
				settings.WrapMode = Epoch::Utils::WrapModeFromString(aNode["Wrap"].as<std::string>(Epoch::Utils::WrapModeToString(TextureWrap::Wrap)));
				settings.AnisotropyLevel = (AnisotropyLevel)aNode["AnisotropyLevel"].as<uint8_t>(uint8_t(0));
				return settings;
			},
			[]() -> ImportSettingsVariant
			{
				return TextureImportSettings();
			}
		);

		ImportSettingsFactory::Register
		(
			AssetType::Model,
			[](const ImportSettingsVariant& variant, YAML::Emitter& out)
			{
				const auto& settings = std::get<ModelImportSettings>(variant);
				out << YAML::Key << "FlattenHierarchy" << YAML::Value << settings.FlattenHierarchy;
			},
			[](const YAML::Node& aNode) -> ImportSettingsVariant
			{
				ModelImportSettings settings;
				settings.FlattenHierarchy = aNode["FlattenHierarchy"].as<bool>(settings.FlattenHierarchy);
				return settings;
			},
			[]() -> ImportSettingsVariant
			{
				return ModelImportSettings();
			}
		);
	}

	void AssetMetadataSerializer::Serialize(const std::filesystem::path& aPath, const AssetMetadata& aMetadata)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Handle" << YAML::Value << aMetadata.Handle;
		out << YAML::Key << "Type" << YAML::Value << Utils::AssetTypeToString(aMetadata.Type);

		out << YAML::Key << "ImportSettings" << YAML::Value << YAML::BeginMap;
		ImportSettingsFactory::Serialize(aMetadata.Type, aMetadata.ImportSettings, out);
		out << YAML::EndMap;

		auto assetManager = AssetManager::GetEditorAssetManager();
		const auto& subAssets = assetManager->GetSubAssets(aMetadata.Handle);
		if (!subAssets.empty())
		{
			out << YAML::Key << "SubAssets" << YAML::Value << YAML::BeginSeq;
			for (const auto& subHandle : subAssets)
			{
				const auto& subMeta = assetManager->GetMetadata(subHandle);
				if (subMeta.IsValid())
				{
					out << YAML::BeginMap;
					out << YAML::Key << "Handle" << YAML::Value << subMeta.Handle;
					out << YAML::Key << "Type" << YAML::Value << Utils::AssetTypeToString(subMeta.Type);
					out << YAML::Key << "Name" << YAML::Value << subMeta.FilePath.string();

					out << YAML::EndMap;
				}
			}
			out << YAML::EndSeq;
		}

		out << YAML::EndMap;

		std::ofstream fout(aPath);
		EPOCH_ASSERT((bool)fout, "Failed to serialize metadata!");

		fout << out.c_str();
	}

	AssetMetadata AssetMetadataSerializer::Deserialize(const std::filesystem::path& aPath)
	{
		std::ifstream stream(aPath);
		EPOCH_ASSERT((bool)stream, "Failed to deserialize metadata!");

		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node node = YAML::Load(strStream.str());

		AssetMetadata metadata;
		metadata.Handle = node["Handle"].as<AssetHandle>();
		metadata.Type = Utils::AssetTypeFromString(node["Type"].as<std::string>());

		if (auto settingsNode = node["ImportSettings"])
		{
			metadata.ImportSettings = ImportSettingsFactory::Deserialize(metadata.Type, settingsNode);
		}

		if (node["SubAssets"])
		{
			auto assetManager = AssetManager::GetEditorAssetManager();
			for (const auto& subNode : node["SubAssets"])
			{
				AssetMetadata subMeta;
				subMeta.Handle = subNode["Handle"].as<AssetHandle>();
				subMeta.Type = Utils::AssetTypeFromString(subNode["Type"].as<std::string>());
				subMeta.FilePath = subNode["Name"].as<std::string>();
				subMeta.IsMemoryAsset = true;

				assetManager->RegisterMetadata(subMeta);
				assetManager->RegisterDependency(metadata.Handle, subMeta.Handle);
			}
		}

		return metadata;
	}
}
