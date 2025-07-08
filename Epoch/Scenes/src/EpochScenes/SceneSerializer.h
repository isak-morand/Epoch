#pragma once
#include <memory>
#include <filesystem>
#include "Scene.h"

namespace Epoch::Scenes
{
	class SceneSerializer
	{
	public:
		SceneSerializer(const std::shared_ptr<Scene>& aScene) : myScene(aScene) {}

		void SerializeText(const std::filesystem::path& aFilepath);
		bool DeserializeText(const std::filesystem::path& aFilepath);

	private:
		std::shared_ptr<Scene> myScene;
	};
}
