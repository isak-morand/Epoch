#pragma once
#include <filesystem>
#include <EpochCore/UUID.h>

namespace Epoch::Projects
{
	struct ProjectSettings
	{
		std::string ProductName = "Unnamed";
		std::string CompanyName = "Unnamed";
		std::string Version = "0.1.0";

		UUID Icon = 0; //Editor only
		UUID StartScene = 0;

		// Not serialized
		std::filesystem::path ProjectFileName;
		std::filesystem::path ProjectDirectory;
	};

	class Project
	{
	public:
		Project() = default;
		~Project() = default;

		std::filesystem::path GetAssetDirectory() const { return mySettings.ProjectDirectory / "Assets"; }

		std::filesystem::path GetGraphicsSettingsPath() const { return mySettings.ProjectDirectory / "Configs" / "Graphics.yaml"; }
		std::filesystem::path GetQualitySettingsPath() const { return mySettings.ProjectDirectory / "Configs" / "Quality.yaml"; }
		std::filesystem::path GetPhysicsSettingsPath() const { return mySettings.ProjectDirectory / "Configs" / "Physics.yaml"; }
		std::filesystem::path GetTimeSettingsPath() const { return mySettings.ProjectDirectory / "Configs" / "Time.yaml"; }

		void Serialize(const std::filesystem::path& aFilepath);
		bool Deserialize(const std::filesystem::path& aFilepath);

		static void SetActive(std::shared_ptr<Project> aProject) { staticActiveProject = aProject; }
		static const Project& GetActive() { return *staticActiveProject; }

	private:
		ProjectSettings mySettings;

		static inline std::shared_ptr<Project> staticActiveProject;
	};
}
