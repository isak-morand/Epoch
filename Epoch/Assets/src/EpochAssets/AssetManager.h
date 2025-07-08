#pragma once
#include "AssetManager/EditorAssetManager.h"

namespace Epoch::Assets
{
	class AssetManager
	{
	public:
		inline static void SetActiveAssetManager(std::shared_ptr<IAssetManager> aAssetManager)
		{
			staticAssetManager = aAssetManager;
		}

		template<typename T>
		static std::shared_ptr<T> GetAsset(TypedAssetHandle<T> aAssetHandle)
		{
			static_assert(std::is_base_of<Asset, T>::value, "GetAsset only works for types derived from Asset");

			EPOCH_ASSERT(staticAssetManager, "AssetManager not set!");

			std::shared_ptr<Asset> asset = staticAssetManager->GetAsset(aAssetHandle.Get());
			return std::dynamic_pointer_cast<T>(asset);
		}

		inline static std::shared_ptr<IAssetManager> GetAssetManager() { return staticAssetManager; }
		inline static std::shared_ptr<EditorAssetManager> GetEditorAssetManager() { return std::static_pointer_cast<EditorAssetManager>(staticAssetManager); }

	private:
		static inline std::shared_ptr<IAssetManager> staticAssetManager;
	};
}
