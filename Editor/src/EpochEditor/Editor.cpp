#include <filesystem>
#include <EpochCore/EntryPoint.h>
#include <EpochSettings/SettingsManager.h>
#include <EpochEngine/Engine.h>
#include <EpochProjects/Project.h>
#include <EpochAssets/AssetManager.h>
#include "EditorSettings.h"
#include "EditorLayer.h"

namespace Epoch
{
	void Main(int argc, char** argv)
	{
		Settings::SettingsManager::Register("Editor", "Settings/EditorSettings.yaml", Editor::EditorSettings::Deserialize, Editor::EditorSettings::Serialize);

		Settings::SettingsManager::Load("Editor");
		const auto& settings = Editor::EditorSettings::Get();

		std::filesystem::path projectPath;
		if (argc > 1)
		{
			projectPath = argv[1];
		}
		else if (settings.LoadLastOpenProject && !settings.LastProjectPath.empty())
		{
			projectPath = settings.LastProjectPath;
		}
		else
		{
			LOG_ERROR_TAG("Editor", "No project path provided and no last project found!");
			std::filesystem::path sandboxProjPath("Sandbox/Sandbox.eproj");
			if (!std::filesystem::exists(sandboxProjPath))
			{
				return;
			}
			LOG_INFO_TAG("Editor", "Defaulting to sandbox project");
			projectPath = sandboxProjPath;
		}

		if (!std::filesystem::exists(projectPath))
		{
			LOG_ERROR_TAG("Editor", "Project '{}' does not exist!", projectPath.string());
			return;
		}

		std::shared_ptr<Projects::Project> project = std::make_shared<Projects::Project>();
		Settings::SettingsManager::Register("Project", projectPath,
			[project](const std::filesystem::path& aFilepath) { return project->Deserialize(aFilepath); },
			[project](const std::filesystem::path& aFilepath) { project->Serialize(aFilepath); });

		if (!Settings::SettingsManager::Load("Project"))
		{
			LOG_ERROR_TAG("Editor", "Failed to deserialize the project '{}'!", projectPath.string());
			return;
		}
		Projects::Project::SetActive(project);

		if (settings.LastProjectPath != projectPath)
		{
			Editor::EditorSettings::Get().LastProjectPath = projectPath.string();
			Settings::SettingsManager::Save("Editor");
		}

		std::shared_ptr<Assets::EditorAssetManager> assetManager = std::make_shared<Assets::EditorAssetManager>();
		Assets::AssetManager::SetActiveAssetManager(assetManager);
		assetManager->Init(project->GetAssetDirectory());

		Epoch::EngineSpecification engineSpec;

#ifdef _DEBUG
		engineSpec.WindowProperties.Title = "My Girlfriend is mad at me if the window isn't titled Something Cute";
#else
		engineSpec.WindowProperties.Title = "Epoch Editor";
#endif // _DEBUG

		engineSpec.WindowProperties.StartHidden = true;
		engineSpec.ImGuiEnabled = true;

		Epoch::Engine engine(engineSpec);

		UUID editorLayerID = 0;

		engine.SetInitCallback
		([&]()
		{
			LayerStack& layerStack = engine.GetLayerStack();
			editorLayerID = layerStack.PushLayer(std::make_unique<Editor::EditorLayer>());
		});

		engine.SetShutdownCallback
		([&]()
		{
			LayerStack& layerStack = engine.GetLayerStack();
			layerStack.PopLayer(editorLayerID);
		});

		engine.Run();
	}
}