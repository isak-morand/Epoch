#include "EditorSettings.h"
#include <fstream>
#include <EpochCore/Log.h>
#include <EpochSerialization/YAMLHelpers.h>

namespace Epoch::Editor
{
	void EditorSettings::Serialize(const std::filesystem::path& aFilepath)
	{
		const auto& settings = EditorSettings::Get();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "EditorSettings" << YAML::Value;

		{
			out << YAML::BeginMap;

			out << YAML::Key << "LoadLastOpenProject" << YAML::Value << settings.LoadLastOpenProject;
			out << YAML::Key << "LastProjectPath" << YAML::Value << settings.LastProjectPath;

			out << YAML::EndMap;
		}

		out << YAML::EndMap;

		std::ofstream fout(aFilepath);
		fout << out.c_str();
		fout.close();
	}

	bool EditorSettings::Deserialize(const std::filesystem::path& aFilepath)
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

		if (!data["EditorSettings"])
		{
			return false;
		}

		YAML::Node rootNode = data["EditorSettings"];

		auto& settings = EditorSettings::Get();
		
		settings.LoadLastOpenProject = rootNode["LoadLastOpenProject"].as<bool>(true);
		settings.LastProjectPath = rootNode["LastProjectPath"].as<std::string>(std::string());

		return true;
	}
}
