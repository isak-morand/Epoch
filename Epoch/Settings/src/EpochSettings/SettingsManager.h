#pragma once
#include <unordered_map>
#include <functional>
#include <filesystem>

namespace Epoch::Settings
{
	class SettingsManager
	{
	public:
		using LoadFunc = std::function<bool(const std::filesystem::path&)>;
		using SaveFunc = std::function<void(const std::filesystem::path&)>;

		static void Register(const std::string& aID, const std::filesystem::path& aFilepath, LoadFunc aLoad, SaveFunc aSave);

		static void Save(const std::string& aID);
		static bool Load(const std::string& aID);

		static void LoadAll();

	private:
		struct SettingsEntry
		{
			std::filesystem::path Filepath;
			LoadFunc LoadFunc;
			SaveFunc SaveFunc;
		};

		static inline std::unordered_map<std::string, SettingsEntry> staticSettings;
	};
}