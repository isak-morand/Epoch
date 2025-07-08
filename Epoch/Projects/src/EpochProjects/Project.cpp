#include "Project.h"
#include <fstream>
#include <EpochCore/Log.h>
#include <EpochSerialization/YAMLHelpers.h>

namespace Epoch::Projects
{
	void Project::Serialize(const std::filesystem::path& aFilepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Project" << YAML::Value;

		{
			out << YAML::BeginMap;

			out << YAML::Key << "ProductName" << YAML::Value << mySettings.CompanyName;
			out << YAML::Key << "CompanyName" << YAML::Value << mySettings.CompanyName;
			out << YAML::Key << "Version" << YAML::Value << mySettings.Version;

			out << YAML::Key << "Icon" << YAML::Value << mySettings.Icon;
			out << YAML::Key << "StartScene" << YAML::Value << mySettings.StartScene;

			out << YAML::EndMap;
		}

		out << YAML::EndMap;

		std::ofstream fileOut(aFilepath);
		fileOut << out.c_str();
		fileOut.close();
	}

	bool Project::Deserialize(const std::filesystem::path& aFilepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(aFilepath.string());
		}
		catch (YAML::ParserException e)
		{
			LOG_ERROR("Failed to load file '{}'\n     {}", aFilepath.string(), e.what());
			return false;
		}

		if (!data["Project"])
		{
			return false;
		}

		YAML::Node rootNode = data["Project"];

		const std::filesystem::path& projectPath = aFilepath;
		mySettings.ProjectFileName = projectPath.filename().string();
		mySettings.ProjectDirectory = projectPath.parent_path().string();

		mySettings.ProductName = rootNode["ProductName"].as<std::string>(mySettings.ProductName);
		mySettings.CompanyName = rootNode["CompanyName"].as<std::string>(mySettings.CompanyName);
		mySettings.Version = rootNode["Version"].as<std::string>(mySettings.Version);

		mySettings.Icon = rootNode["Icon"].as<UUID>(mySettings.Icon);
		mySettings.StartScene = rootNode["StartScene"].as<UUID>(mySettings.StartScene);

		return true;
	}
}
