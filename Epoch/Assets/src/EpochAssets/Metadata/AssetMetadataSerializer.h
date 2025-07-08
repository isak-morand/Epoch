#pragma once
#include "AssetMetadata.h"

namespace YAML
{
	class Emitter;
	class Node;
}

namespace Epoch::Assets
{
	class ImportSettingsFactory
	{
	public:
		using ImportSettingsSerializerFn = std::function<void(const ImportSettingsVariant&, YAML::Emitter&)>;
		using ImportSettingsDeserializerFn = std::function<ImportSettingsVariant(const YAML::Node&)>;
		using ImportSettingsCreatorFn = std::function<ImportSettingsVariant()>;

	public:
		static void Register(AssetType aType, ImportSettingsSerializerFn aSerializer, ImportSettingsDeserializerFn aDeserializer, ImportSettingsCreatorFn aCreator);

		static void Serialize(AssetType aType, const ImportSettingsVariant& aSettings, YAML::Emitter& out);
		static ImportSettingsVariant Deserialize(AssetType aType, const YAML::Node& aNode);
		static ImportSettingsVariant CreateDefault(AssetType aType);

	private:
		static inline std::unordered_map<AssetType, ImportSettingsSerializerFn> staticSerializers;
		static inline std::unordered_map<AssetType, ImportSettingsDeserializerFn> staticDeserializers;
		static inline std::unordered_map<AssetType, ImportSettingsCreatorFn> staticCreators;
	};

	class AssetMetadataSerializer
	{
	public:
		static void Init();

		static void Serialize(const std::filesystem::path& aPath, const AssetMetadata& aMetadata);
		static AssetMetadata Deserialize(const std::filesystem::path& aPath);
	};
}
