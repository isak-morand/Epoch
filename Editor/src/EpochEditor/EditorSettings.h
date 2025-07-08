#pragma once
#include <map>
#include <ctime>
#include <string>
#include <filesystem>

namespace Epoch::Editor
{
	struct EditorSettings
	{
		bool LoadLastOpenProject = true;
		std::string LastProjectPath = "";

		static void Serialize(const std::filesystem::path& aFilepath);
		static bool Deserialize(const std::filesystem::path& aFilepath);

		static EditorSettings& Get() { static EditorSettings staticSettings; return staticSettings; }
	};
}
