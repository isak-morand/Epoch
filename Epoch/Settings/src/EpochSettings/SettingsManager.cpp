#include "SettingsManager.h"
#include <EpochCore/FileSystem.h>

namespace Epoch::Settings
{
	void SettingsManager::Register(const std::string& aID, const std::filesystem::path& aFilepath, LoadFunc aLoad, SaveFunc aSave)
	{
		staticSettings[aID] = { aFilepath, std::move(aLoad), std::move(aSave) };
	}

	void SettingsManager::Save(const std::string& aID)
	{
		if (auto it = staticSettings.find(aID); it != staticSettings.end())
		{
			if (!Core::FileSystem::Exists(it->second.Filepath.parent_path()))
			{
				Core::FileSystem::CreateDirectory(it->second.Filepath.parent_path());
			}
			it->second.SaveFunc(it->second.Filepath);
		}
	}

	bool SettingsManager::Load(const std::string& aID)
	{
		if (auto it = staticSettings.find(aID); it != staticSettings.end())
		{
			if (!Core::FileSystem::Exists(it->second.Filepath))
			{
				Save(aID);
				return true;
			}
			else
			{
				return it->second.LoadFunc(it->second.Filepath);
			}
		}
		return false;
	}

	void SettingsManager::LoadAll()
	{
		for (const auto& [id, _] : staticSettings)
		{
			Load(id);
		}
	}
}
