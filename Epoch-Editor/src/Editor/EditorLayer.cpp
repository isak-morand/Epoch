#include "EditorLayer.h"
#include <CommonUtilities/Timer.h>
#include <CommonUtilities/Input/InputHandler.h>
#include <CommonUtilities/StringUtils.h>
#include <CommonUtilities/Math/Random.h>
#include <Epoch/Debug/Log.h>
#include <Epoch/Debug/Profiler.h>
#include <Epoch/Utils/FileSystem.h>
#include <Epoch/ImGui/ImGui.h>
#include <Epoch/ImGui/UICore.h>
#include <Epoch/ImGui/ImGuizmo.h>
#include <Epoch/Core/Application.h>
#include <Epoch/Core/Window.h>
#include <Epoch/Core/Input.h>
#include <Epoch/Core/Events/KeyEvent.h>
#include <Epoch/Core/Events/MouseEvent.h>
#include <Epoch/Core/Events/WindowEvent.h>
#include <Epoch/Core/Events/SceneEvents.h>
#include <Epoch/Rendering/Renderer.h>
#include <Epoch/Rendering/DebugRenderer.h>
#include <Epoch/Scene/SceneRenderer2D.h>
#include <Epoch/Project/ProjectSerializer.h>
#include <Epoch/Project/RuntimeBuilder.h>
#include <Epoch/Scene/SceneSerializer.h>
#include <Epoch/Scene/Prefab.h>
#include <Epoch/Script/ScriptEngine.h>
#include <Epoch/Script/ScriptBuilder.h>
#include <Epoch/Editor/SelectionManager.h>
#include <Epoch/Editor/GradientEditor.h>
#include <Epoch/Editor/EditorSettings.h>
#include <Epoch/Editor/Console/EditorConsolePanel.h>
#include <Epoch/Editor/PanelIDs.h>

#include <Epoch/Embed/OpenSans_Regular.embed>
#include <Epoch/Embed/OpenSans_Bold.embed>
#include <Epoch/Embed/Fontawesome.embed>

#include "EditorResources.h"
#include "Panels/ViewportPanel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/AssetManagerPanel.h"
#include "Panels/PreferencesPanel.h"
#include "Panels/ProjectSettingsPanel.h"
#include "Panels/ShaderLibraryPanel.h"
#include "Panels/StatisticsPanel.h"
#include "Panels/ScriptEngineDebugPanel.h"

namespace Epoch
{
	static bool staticCreateNewProj = false;

	EditorLayer::EditorLayer() : Layer("Editor Layer")
	{
	}

	void EditorLayer::OnAttach()
	{
		EPOCH_PROFILE_FUNC();

		EditorSettingsSerializer::Init();
		EditorResources::Init();

		JobSystem& js = Application::Get().GetJobSystem();
		js.WaitUntilDone();

		//Set double clicked highlight callbacks
		//UI::OnAssetReferenceDoubleClickedCallback = [](AssetHandle aAssetHandle)
		//	{
		//		SelectionManager::DeselectAll(SelectionContext::Asset);
		//		SelectionManager::Select(SelectionContext::Asset, aAssetHandle);
		//	};
		//
		//UI::OnEntityReferenceDoubleClickedCallback = [](UUID aEntity)
		//	{
		//		SelectionManager::DeselectAll(SelectionContext::Entity);
		//		SelectionManager::Select(SelectionContext::Entity, aEntity);
		//	};

		myPanelManager = std::make_unique<PanelManager>();
		myPanelManager->SetEntityDestroyedCallback([this](Entity aEntity) { OnEntityDeleted(aEntity); });

		myGameViewport = myPanelManager->AddPanel<ViewportPanel>(PanelCategory::View, "Panel/Game", GAME_PANEL_ID, true);

		mySceneViewport = myPanelManager->AddPanel<ViewportPanel>(PanelCategory::View, "Panel/Scene", SCENE_PANEL_ID, true);
		mySceneViewport->AddAdditionalFunction([this]() { UpdateViewportBoxSelection(); });
		mySceneViewport->AddAdditionalFunction([this]() { DrawGizmos(); });
		mySceneViewport->AddAdditionalFunction([this]() { HandleAssetDrop(); });
		mySceneViewport->AddAdditionalFunction([this]() { DisplayColorGradingLUT(); });

		std::shared_ptr<SceneHierarchyPanel> sceneHierarchyPanel = myPanelManager->AddPanel<SceneHierarchyPanel>(PanelCategory::View, "Panel/Scene Hierarchy", SCENE_HIERARCHY_PANEL_ID, true);
		sceneHierarchyPanel->SetEntityCreationCallback([this](Entity aEntity) { return OnEntityCreated(aEntity); });
		sceneHierarchyPanel->AddEntityPopupPlugin("Snap to Editor Camera", [this](Entity aEntity) { OnSnapToEditorCamera(aEntity); });
		//sceneHierarchyPanel->AddEntityPopupPlugin("Reset Bone Transforms", [this](Entity aEntity) { OnResetBoneTransforms(aEntity); });

		std::shared_ptr<ContentBrowserPanel> contentBrowserPanel = myPanelManager->AddPanel<ContentBrowserPanel>(PanelCategory::View, "Panel/Content Browser", CONTENT_BROWSER_PANEL_ID, true);

		contentBrowserPanel->RegisterItemActivateCallbackForType(AssetType::Scene, [this](const AssetMetadata& aMetadata)
			{
				OpenScene(Project::GetEditorAssetManager()->GetFileSystemPath(aMetadata));
			});

		contentBrowserPanel->RegisterItemActivateCallbackForType(AssetType::ScriptFile, [this](const AssetMetadata& aMetadata)
			{
				FileSystem::OpenExternally(Project::GetEditorAssetManager()->GetFileSystemPath(aMetadata));
			});

		contentBrowserPanel->RegisterNewAssetCreatedCallbackForType(AssetType::ScriptFile, [this](const AssetMetadata& aMetadata)
			{
				ScriptBuilder::RegenerateScriptSolution(Project::GetProjectDirectory());
			});

		contentBrowserPanel->RegisterAssetDeletedCallbackForType(AssetType::ScriptFile, [this](const AssetMetadata& aMetadata)
			{
				ScriptBuilder::RegenerateScriptSolution(Project::GetProjectDirectory());
			});

		contentBrowserPanel->RegisterAssetRenamedCallbackForType(AssetType::ScriptFile, [this](const AssetMetadata& aMetadata)
			{
				ScriptBuilder::RegenerateScriptSolution(Project::GetProjectDirectory());
			});

		contentBrowserPanel->RegisterAssetRenamedCallbackForType(AssetType::Scene, [this](const AssetMetadata& aMetadata)
			{
				if (aMetadata.handle == myEditorScene->GetHandle())
				{
					OnCurrentSceneRenamed(aMetadata);
				}
			});

		myPanelManager->AddPanel<ProjectSettingsPanel>(PanelCategory::Edit, "Project Settings", PROJECT_SETTINGS_PANEL_ID, false);
		myPanelManager->AddPanel<PreferencesPanel>(PanelCategory::Edit, "Preferences", PREFERENCES_PANEL_ID, false);

		myPanelManager->AddPanel<InspectorPanel>(PanelCategory::View, "Panel/Inspector", INSPECTOR_PANEL_ID, true);
		myPanelManager->AddPanel<EditorConsolePanel>(PanelCategory::View, "Panel/Console", CONSOLE_PANEL_ID, true);
		myPanelManager->AddPanel<ShaderLibraryPanel>(PanelCategory::View, "Panel/Shader Library", SHADER_LIBRARY_PANEL_ID, false);
		myPanelManager->AddPanel<AssetManagerPanel>(PanelCategory::View, "Debug/Asset Manager", ASSET_MANAGER_PANEL_ID, false);
		myPanelManager->AddPanel<ScriptEngineDebugPanel>(PanelCategory::View, "Debug/Script Engine", SCRIPT_ENGINE_DEBUG_PANEL_ID, false);
		myStatisticsPanel = myPanelManager->AddPanel<StatisticsPanel>(PanelCategory::View, "Debug/Statistics", STATISTICS_PANEL_ID, false);

		myPanelManager->Deserialize();

		const std::string& lastProjectPath = EditorSettings::Get().lastProjectPath;
		const bool openLastProject = EditorSettings::Get().loadLastOpenProject && !lastProjectPath.empty() && FileSystem::Exists(lastProjectPath);
		if (openLastProject)
		{
			OpenProject(lastProjectPath);
		}
		else if (FileSystem::Exists("DefaultProject/DefaultProject.eproj"))
		{
			OpenProject("DefaultProject/DefaultProject.eproj");
		}
		else
		{
			Application::Get().Close();
		}

		{
			EPOCH_PROFILE_SCOPE("Load fonts");

			UI::Fonts::Add("Regular", Buffer(OpenSans_Regular, OpenSans_Regular_len), true);

			static const ImWchar staticFontAwesomeRanges[] = { EP_ICON_MIN, EP_ICON_MAX, 0 };
			UI::FontConfiguration fontAwesome;
			fontAwesome.fontName = "FontAwesome";
			fontAwesome.data = Buffer(fontawesome, fontawesome_len);
			fontAwesome.size = 16.0f;
			fontAwesome.glyphRanges = staticFontAwesomeRanges;
			fontAwesome.mergeWithLast = true;
			UI::Fonts::Add(fontAwesome);

			UI::Fonts::Add("Bold", Buffer(OpenSans_Bold, OpenSans_Bold_len));
		}

		GradientEditor::Get().Init();

		//Debug Renderer
		{
			auto sceneRenderer = mySceneViewport->GetSceneRenderer();
			myDebugRenderer = std::make_shared<DebugRenderer>();
			myDebugRenderer->Init(sceneRenderer->GetExternalCompositingFramebuffer());
			sceneRenderer->SetDebugRenderer(myDebugRenderer);

			myStatisticsPanel->SetSceneRenderer(sceneRenderer);
			myStatisticsPanel->SetDebugRenderer(myDebugRenderer);
		}
	}

	void EditorLayer::OnDetach()
	{
		EPOCH_PROFILE_FUNC();

		if (mySceneState != SceneState::Edit) OnSceneStop();

		if (Project::GetActive())
		{
			CloseProject();
		}

		EditorSettingsSerializer::SaveSettings();
	}

	void EditorLayer::OnUpdate()
	{
		EPOCH_PROFILE_FUNC();

		if (!myActiveScene) return;

		myEditorCamera.SetActive(mySceneViewport->AllowEditorCameraMovement());
		myEditorCamera.OnUpdate();

		switch (mySceneState)
		{
		case SceneState::Edit:
		case SceneState::Simulate:
		{
			myActiveScene->OnUpdateEditor();
			break;
		}
		case SceneState::Play:
		{
			myActiveScene->OnUpdateRuntime();

			Entity camEntity = myActiveScene->GetPrimaryCameraEntity();
			if (camEntity)
			{
				{
					EPOCH_PROFILE_SCOPE("C# OnDebug");

					auto entities = myActiveScene->GetAllEntitiesWith<ScriptComponent>();
					for (auto id : entities)
					{
						Entity entity = Entity(id, myActiveScene.get());
						const auto& sc = entities.get<ScriptComponent>(id);
						if (entity.IsActive() && sc.isActive && sc.IsFlagSet(ManagedClassMethodFlags::ShouldDebug) && ScriptEngine::IsEntityInstantiated(entity))
						{
							ScriptEngine::CallMethod(ScriptEngine::GetEntityInstance(entity.GetUUID()), "OnDebug");
						}
					}
				}
			}

			break;
		}
		}

		// Render scene view
		if (mySceneViewport->IsVisible())
		{
			auto sceneRenderer = mySceneViewport->GetSceneRenderer();

			Scene::EditorRenderSettings renderSetting;
			renderSetting.cullWithGameCamera = myCullWithSceneCamera;
			renderSetting.postProcessingEnabled = myPostProcessingInSceneView;
			renderSetting.displayUI = myDisplayUIInSceneView;

			sceneRenderer->SetScene(myActiveScene);
			myActiveScene->OnRenderEditor(sceneRenderer, myEditorCamera, renderSetting);

			OnRenderOverlay();
		}
		
		// Render game view
		if (myGameViewport->IsVisible())
		{
			auto sceneRenderer = myGameViewport->GetSceneRenderer();

			sceneRenderer->SetScene(myActiveScene);
			myActiveScene->OnRenderGame(sceneRenderer);
		}

		switch (mySceneState)
		{
		case SceneState::Edit:
		{
			if (mySceneState == SceneState::Edit && EditorSettings::Get().autosaveEnabled)
			{
				myTimeSinceLastSave += CU::Timer::GetDeltaTime();
				if (myTimeSinceLastSave >= EditorSettings::Get().autosaveIntervalSeconds)
				{
					SaveSceneAuto();
				}
			}
			break;
		}
		case SceneState::Play:
		{
			Entity camEntity = myActiveScene->GetPrimaryCameraEntity();
			if (camEntity)
			{
				CU::Transform worldTrans = camEntity.GetWorldSpaceTransform();
				const CameraComponent& camComponent = camEntity.GetComponent<CameraComponent>();
				myDebugRenderer->Render(worldTrans.GetMatrix().GetFastInverse(), camComponent.camera.GetProjectionMatrix(), myDebugRendererOnTop);
			}

			auto sceneUpdateQueue = myPostSceneUpdateQueue;
			myPostSceneUpdateQueue.clear();
			for (auto& fn : sceneUpdateQueue)
			{
				fn();
			}
			break;
		}
		}

		//switch (mySceneState)
		//{
		//case SceneState::Edit:
		//case SceneState::Simulate:
		//{
		//	auto sceneRenderer = mySceneViewport->GetSceneRenderer();
		//	myEditorCamera.SetActive(mySceneViewport->AllowEditorCameraMovement());
		//	myEditorCamera.OnUpdate();
		//	myActiveScene->OnUpdateEditor();
		//	sceneRenderer->SetScene(myActiveScene);
		//	myActiveScene->OnRenderEditor(sceneRenderer, myEditorCamera, !myCullWithSceneCamera);
		//
		//	OnRenderOverlay();
		//
		//	if (mySceneState == SceneState::Edit && EditorSettings::Get().autosaveEnabled)
		//	{
		//		myTimeSinceLastSave += CU::Timer::GetDeltaTime();
		//		if (myTimeSinceLastSave >= EditorSettings::Get().autosaveIntervalSeconds)
		//		{
		//			SaveSceneAuto();
		//		}
		//	}
		//	break;
		//}
		//case SceneState::Play:
		//{
		//	auto sceneRenderer = myGameViewport->GetSceneRenderer();
		//	myActiveScene->OnUpdateRuntime();
		//	sceneRenderer->SetScene(myActiveScene);
		//	myActiveScene->OnRenderRuntime(sceneRenderer);
		//
		//	Entity camEntity = myActiveScene->GetPrimaryCameraEntity();
		//	if (camEntity)
		//	{
		//		{
		//			EPOCH_PROFILE_SCOPE("C# OnDebug");
		//
		//			auto entities = myActiveScene->GetAllEntitiesWith<ScriptComponent>();
		//			for (auto id : entities)
		//			{
		//				Entity entity = Entity(id, myActiveScene.get());
		//				const auto& sc = entities.get<ScriptComponent>(id);
		//				if (entity.IsActive() && sc.isActive && sc.IsFlagSet(ManagedClassMethodFlags::ShouldDebug) && ScriptEngine::IsEntityInstantiated(entity))
		//				{
		//					ScriptEngine::CallMethod(ScriptEngine::GetEntityInstance(entity.GetUUID()), "OnDebug");
		//				}
		//			}
		//		}
		//
		//		CU::Transform worldTrans = camEntity.GetWorldSpaceTransform();
		//		const CameraComponent& camComponent = camEntity.GetComponent<CameraComponent>();
		//		myDebugRenderer->Render(worldTrans.GetMatrix().GetFastInverse(), camComponent.camera.GetProjectionMatrix(), myDebugRendererOnTop);
		//	}
		//
		//	auto sceneUpdateQueue = myPostSceneUpdateQueue;
		//	myPostSceneUpdateQueue.clear();
		//	for (auto& fn : sceneUpdateQueue)
		//	{
		//		fn();
		//	}
		//	break;
		//}
		//}

		if (ScriptEngine::ShouldReloadAppAssembly())
		{
			bool reload = true;

			if (mySceneState != SceneState::Edit)
			{
				if (EditorSettings::Get().reloadScriptAssemblyWhilePlaying == ReloadScriptAssemblyWhilePlaying::Stop)
				{
					OnSceneStop();
					reload = true;
				}
				else
				{
					reload = false;
				}
			}

			if (reload)
			{
				ScriptEngine::ReloadAppAssembly();
			}
		}

		if (Input::IsMouseButtonHeld(MouseButton::Right) && !mySceneViewport->GetStartedCameraClickInPanel() && mySceneViewport->IsFocused() && mySceneViewport->IsHovered())
		{
			mySceneViewport->SetStartedCameraClickInPanel(true);
		}
		else if ((!Input::IsMouseButtonHeld(MouseButton::Right) || !mySceneViewport->IsHovered()) && mySceneViewport->GetStartedCameraClickInPanel())
		{
			mySceneViewport->SetStartedCameraClickInPanel(false);
		}
	}

	void EditorLayer::OnImGuiRender()
	{
		EPOCH_PROFILE_FUNC();

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || (ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
		{
			if (!mySceneViewport->GetStartedCameraClickInPanel() && mySceneViewport->IsHovered())
			{
				ImGui::FocusWindow(GImGui->HoveredWindow);
				Input::SetCursorMode(CursorMode::Normal);
			}
		}

		OnRenderMenuBar();

		BeginDockspace();

		if (staticCreateNewProj)
		{
			staticCreateNewProj = false;
			ImGui::OpenPopup("Create Project");
		}
		ShowCreateProjectPopup(); // TODO: Make a easier way to display popups

		myPanelManager->OnImGuiRender();

		ToolbarPanel();
		myEditorCamera.SetViewportSize(mySceneViewport->Size().x, mySceneViewport->Size().y);
		myDebugRenderer->SetViewportSize(mySceneViewport->Size().x, mySceneViewport->Size().y);
		myActiveScene->SetViewportSize(myGameViewport->Size().x, myGameViewport->Size().y);

		auto [mx, my] = myGameViewport->GetMouseViewportCord();
		myActiveScene->SetMousePos({ mx, my });

		EditorOptionsPanel();

		EndDockspace();

		//ImGui::ShowDemoWindow();
	}

	void EditorLayer::OnEvent(Event& aEvent)
	{
		myPanelManager->OnEvent(aEvent);

		if (aEvent.IsHandled())
		{
			return;
		}

		EventDispatcher dispatcher(aEvent);
		dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& aEvent) { return OnKeyPressedEvent(aEvent); });
		dispatcher.Dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent& aEvent) { return OnMouseButtonPressed(aEvent); });
	}

	void EditorLayer::BeginDockspace()
	{
		// Note: Switch this to true to enable dockspace
		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistent = true;
		bool opt_fullscreen = opt_fullscreen_persistent;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dock able into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		{
			window_flags |= ImGuiWindowFlags_NoBackground;
		}

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 12.0f));
		ImGui::Begin("DockSpace Editor", &dockspaceOpen, window_flags);
		//ImGui::PopStyleVar(2);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
		{
			ImGui::PopStyleVar(2);
		}

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		//float minWinSizeX = style.WindowMinSize.x;
		//float minWinSizeY = style.WindowMinSize.y;
		style.WindowMinSize.x = 100.0f;
		style.WindowMinSize.y = 30.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		//style.WindowMinSize.x = minWinSizeX;
		//style.WindowMinSize.y = minWinSizeY;
	}

	void EditorLayer::EndDockspace()
	{
		ImGui::End();
	}

	void EditorLayer::OnRenderMenuBar()
	{
		ImVec2 framePadding = ImGui::GetStyle().FramePadding;
		//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 12.0f));
		if (ImGui::BeginMainMenuBar())
		{
			ImVec2 pos = ImGui::GetCursorPos();
			float menuBarCenter = ImGui::GetContentRegionAvail().x / 2;

			if (ImGui::BeginMenu("File"))
			{
				{
					const bool projectOpen = !(!Project::GetActive());

					if (ImGui::MenuItem("Open Scene...", "Ctrl+O", false, projectOpen))
					{
						OpenScene();
					}

					const auto& projectUserConf = Project::GetActive()->GetUserConfig();
					if (ImGui::BeginMenu("Open Recent Scene...", projectUserConf.recentScenes.size() > 1))
					{
						size_t i = 0;
						for (auto it = projectUserConf.recentScenes.begin(); it != projectUserConf.recentScenes.end(); it++)
						{
							if (i > 10) break;
							if (it->second.filePath == myEditorScenePath) continue;

							if (ImGui::MenuItem(it->second.name.c_str()))
							{
								OpenScene(it->second.filePath);

								break;
							}

							i++;
						}
						ImGui::EndMenu();
					}

					if (ImGui::MenuItem("New Scene", "Ctrl+N", false, projectOpen))
					{
						NewScene();
					}

					if (ImGui::MenuItem("Save Scene", "Ctrl+S", false, projectOpen))
					{
						SaveScene();
					}

					if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S", false, projectOpen))
					{
						SaveSceneAs();
					}
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Open Project..."))
				{
					OpenProject();
				}

				if (ImGui::BeginMenu("Open Recent Project...", EditorSettings::Get().recentProjects.size() > 1))
				{
					size_t i = 0;
					for (auto it = EditorSettings::Get().recentProjects.begin(); it != EditorSettings::Get().recentProjects.end(); it++)
					{
						if (i > 10) break;
						if (it->second.filePath == Project::GetProjectPath()) continue;

						if (ImGui::MenuItem(it->second.name.c_str()))
						{
							// stash filepath away and defer actual opening of project until it is "safe" to do so
							//strcpy(s_OpenProjectFilePathBuffer, it->second.FilePath.data());

							OpenProject(it->second.filePath);

							break;
						}

						i++;
					}
					ImGui::EndMenu();
				}

				if (ImGui::MenuItem("Create Project"))
				{
					staticCreateNewProj = true;
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Build"))
				{
					const auto buildLocation = FileSystem::OpenFolderDialog();

					if (!buildLocation.empty())
					{
						RuntimeBuilder::Build(buildLocation);
					}
				}

				if (ImGui::MenuItem("Build (Dev)"))
				{
					const auto buildLocation = FileSystem::OpenFolderDialog();

					if (!buildLocation.empty())
					{
						RuntimeBuilder::Build(buildLocation, true);
					}
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Exit"))
				{
					Application::Get().Close();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				for (auto& [id, panelData] : myPanelManager->GetPanels(PanelCategory::Edit))
				{
					auto nameParts = CU::SplitString(panelData.menuName, "/");
					RecursivePanelMenuItem(nameParts, 0, panelData.isOpen);
				}
				ImGui::Separator();

				const std::filesystem::path scriptSolutionPath = Project::GetScriptSolutionPath();
				if (ImGui::MenuItem("Open C# Solution", nullptr, nullptr, FileSystem::Exists(scriptSolutionPath)))
				{
					FileSystem::OpenExternally(scriptSolutionPath);
				}

				if (ImGui::BeginMenu("C# Project"))
				{
					if (ImGui::MenuItem("Regenerate Solution", nullptr, nullptr, FileSystem::Exists(scriptSolutionPath)))
					{
						ScriptBuilder::RegenerateScriptSolution(Project::GetProjectDirectory());
					}

					if (ImGui::MenuItem("Build Assembly", nullptr, nullptr, FileSystem::Exists(scriptSolutionPath)))
					{
						ScriptBuilder::BuildScriptAssembly();
					}

					if (ImGui::MenuItem("Reload Assembly"))
					{
						ScriptEngine::ReloadAppAssembly();
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				for (auto& [id, panelData] : myPanelManager->GetPanels(PanelCategory::View))
				{
					auto nameParts = CU::SplitString(panelData.menuName, "/");
					RecursivePanelMenuItem(nameParts, 0, panelData.isOpen);
				}

				ImGui::Separator();

				if (ImGui::BeginMenu("Layout"))
				{
					std::filesystem::path layoutDir = "Resources/Layouts";
					for (auto const& dirEntry : std::filesystem::directory_iterator(layoutDir))
					{
						if (ImGui::MenuItem(dirEntry.path().stem().string().c_str()))
						{
							ImGui::LoadIniSettingsFromDisk(dirEntry.path().string().c_str());
						}
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			if (false)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				auto& colors = ImGui::GetStyle().Colors;
				const ImVec4& buttonHovered = colors[ImGuiCol_ButtonHovered];
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
				const auto& buttonActive = colors[ImGuiCol_ButtonActive];
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

				ImGui::SetCursorPos(pos + ImVec2(menuBarCenter, ImGui::GetContentRegionAvail().y / 2 - 11));
				auto icon = (mySceneState == SceneState::Edit) ? EditorResources::PlayButton : EditorResources::StopButton;
				if (ImGui::ImageButton((ImTextureID)icon->GetView(), ImVec2(24, 24), { 0, 0 }, { 1, 1 }, (int)framePadding.y))
				{
					if (mySceneState == SceneState::Edit)
					{
						OnScenePlay();
					}
					else if (mySceneState == SceneState::Play)
					{
						OnSceneStop();
					}
				}
				if (ImGui::BeginItemTooltip())
				{
					(mySceneState == SceneState::Edit) ? ImGui::Text(" Play ") : ImGui::Text(" Stop ");
					ImGui::EndTooltip();
				}

				ImGui::PopStyleVar();
				ImGui::PopStyleColor(3);
			}

			ImGui::EndMainMenuBar();
		}
		//ImGui::PopStyleVar();
	}

	void EditorLayer::RecursivePanelMenuItem(const std::vector<std::string>& aNameParts, uint32_t aDepth, bool& aIsOpen)
	{
		if (aDepth < aNameParts.size() - 1)
		{
			if (ImGui::BeginMenu(aNameParts[aDepth].c_str()))
			{
				RecursivePanelMenuItem(aNameParts, ++aDepth, aIsOpen);
				ImGui::EndMenu();
			}
		}
		else
		{
			if (ImGui::MenuItem(aNameParts.back().c_str(), nullptr, &aIsOpen))
			{
				myPanelManager->Serialize();
			}
		}
	}

	Entity EditorLayer::GetHoveredEntity()
	{
		auto [px, py] = mySceneViewport->GetMouseViewportCord();

		auto sceneRenderer = mySceneViewport->GetSceneRenderer();
		auto IDBuffer = sceneRenderer->GetEntityIDTexture();
		auto pixelData = IDBuffer->ReadData(1, 1, (uint32_t)px, (uint32_t)py);
		if (!pixelData)
		{
			return Entity();
		}

		uint32_t id = *pixelData.As<uint32_t>();
		pixelData.Release();

		if (id == 0)
		{
			return Entity();
		}

		return Entity((entt::entity)(id - 1), myActiveScene.get());
	}

	void EditorLayer::HandleAssetDrop()
	{
		if (mySceneState == SceneState::Play) return;
		if (!ImGui::BeginDragDropTarget()) return;

		auto data = ImGui::AcceptDragDropPayload("asset_payload");
		if (data)
		{
			bool grabFocus = false;

			unsigned count = data->DataSize / sizeof(AssetHandle);

			for (unsigned i = 0; i < count; i++)
			{
				AssetHandle assetHandle = *(((AssetHandle*)data->Data) + i);
				const AssetMetadata& assetData = Project::GetEditorAssetManager()->GetMetadata(assetHandle);

				if (assetData.type == AssetType::Scene)
				{
					OpenScene(Project::GetAssetDirectory() / assetData.filePath);
					break;
				}

				std::shared_ptr<Asset> asset = AssetManager::GetAsset<Asset>(assetHandle);

				if (!asset)
				{
					continue;
				}

				if (asset->GetAssetType() == AssetType::Mesh)
				{
					std::shared_ptr<Mesh> mesh = std::static_pointer_cast<Mesh>(asset);
					Entity meshEntity = myActiveScene->InstantiateMesh(mesh);

					OnEntityCreated(meshEntity);

					SelectionManager::DeselectAll(SelectionContext::Scene);
					SelectionManager::Select(SelectionContext::Scene, meshEntity.GetUUID());

					grabFocus = true;
				}
				else if (asset->GetAssetType() == AssetType::Texture)
				{
					if (Input::IsKeyHeld(KeyCode::LeftShift))
					{
						Entity hoveredEntity = GetHoveredEntity();
						if (!hoveredEntity)
						{
							continue;
						}

						if (!hoveredEntity.HasComponent<MeshRendererComponent>())
						{
							continue;
						}

						MeshRendererComponent& mrc = hoveredEntity.GetComponent<MeshRendererComponent>();
						if (mrc.materialTable->GetMaterialCount() > 0)
						{
							AssetHandle materialHandle = mrc.materialTable->GetMaterial(0);
							if (materialHandle != Hash::GenerateFNVHash("Default-Material"))
							{
								auto material = AssetManager::GetAsset<Material>(materialHandle);
								material->SetAlbedoTexture(assetHandle);
							}
						}
					}
					else
					{
						Entity spriteEntity = myActiveScene->CreateEntity(assetData.filePath.stem().string());
						spriteEntity.AddComponent<SpriteRendererComponent>(assetHandle);

						OnEntityCreated(spriteEntity);

						SelectionManager::DeselectAll(SelectionContext::Scene);
						SelectionManager::Select(SelectionContext::Scene, spriteEntity.GetUUID());
					}

					grabFocus = true;
				}
				else if (asset->GetAssetType() == AssetType::Prefab)
				{
					std::shared_ptr<Prefab> prefab = std::static_pointer_cast<Prefab>(asset);
					Entity prefabEntity = myActiveScene->Instantiate(prefab);

					if (!EditorSettings::Get().createEntitiesAtOrigin)
					{
						auto& tc = prefabEntity.GetComponent<TransformComponent>();
						tc.transform.SetTranslation(myEditorCamera.GetTransform().GetTranslation() + myEditorCamera.GetTransform().GetForward() * 500.0f);
					}
					else
					{
						auto& tc = prefabEntity.GetComponent<TransformComponent>();
						tc.transform.SetTranslation(CU::Vector3f::Zero);
					}

					SelectionManager::DeselectAll(SelectionContext::Scene);
					SelectionManager::Select(SelectionContext::Scene, prefabEntity.GetUUID());

					grabFocus = true;
				}
				else if (asset->GetAssetType() == AssetType::Material)
				{
					Entity hoveredEntity = GetHoveredEntity();
					if (!hoveredEntity)
					{
						continue;
					}

					if (!hoveredEntity.HasComponent<MeshRendererComponent>())
					{
						continue;
					}

					MeshRendererComponent& mrc = hoveredEntity.GetComponent<MeshRendererComponent>();
					if (mrc.materialTable->GetMaterialCount() > 0)
					{
						mrc.materialTable->SetMaterial(0, assetHandle);
					}
					else
					{
						mrc.materialTable->AddMaterial(assetHandle);
					}

					grabFocus = true;
				}
			}

			if (grabFocus) mySceneViewport->SetFocus();
		}

		ImGui::EndDragDropTarget();
	}

	void EditorLayer::ShowCreateProjectPopup()
	{
		if (ImGui::BeginPopupModal("Create Project", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			static char* projectNameBuffer = new char[255];
			static char* newProjectFilePathBuffer = new char[512];
			static bool inited = false;
			if (!inited)
			{
				inited = true;
				memset(projectNameBuffer, 0, 255);
				memset(newProjectFilePathBuffer, 0, 512);
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 7));

			UI::Fonts::PushFont("Bold");
			std::string fullProjectPath = strlen(newProjectFilePathBuffer) > 0 ? std::string(newProjectFilePathBuffer) + "\\" + std::string(projectNameBuffer) : "";
			ImGui::Text("Full Project Path:");
			UI::Fonts::PopFont();
			ImGui::SameLine();
			ImGui::Text(fullProjectPath.c_str());

			ImGui::SetNextItemWidth(500);
			ImGui::InputTextWithHint("##new_project_name", "Project Name...", projectNameBuffer, 255);

			ImVec2 label_size = ImGui::CalcTextSize("...", NULL, true);
			auto& style = ImGui::GetStyle();
			ImVec2 button_size = ImGui::CalcItemSize(ImVec2(0, 0), label_size.x + style.FramePadding.x + style.ItemInnerSpacing.x, label_size.y + style.FramePadding.y);

			ImGui::SetNextItemWidth(500 - button_size.x - style.FramePadding.x - style.ItemInnerSpacing.x);
			ImGui::InputTextWithHint("##new_project_location", "Project Location...", newProjectFilePathBuffer, 512);

			ImGui::SameLine();

			if (ImGui::Button("..."))
			{
				std::string result = FileSystem::OpenFolderDialog().string();
				memcpy(newProjectFilePathBuffer, result.data(), result.length());
			}

			if (ImGui::Button("Create", ImVec2(75, 0)))
			{
				if (CU::EndsWith(newProjectFilePathBuffer, "\\"))
				{
					CreateProject(std::string(newProjectFilePathBuffer) + std::string(projectNameBuffer));
				}
				else
				{
					CreateProject(fullProjectPath);
				}

				memset(projectNameBuffer, 0, 255);
				memset(newProjectFilePathBuffer, 0, 512);
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(75, 0)))
			{
				memset(projectNameBuffer, 0, 255);
				memset(newProjectFilePathBuffer, 0, 512);
				ImGui::CloseCurrentPopup();
			}

			ImGui::PopStyleVar();
			ImGui::EndPopup();
		}
	}

	void EditorLayer::OnRenderOverlay()
	{
		EPOCH_PROFILE_FUNC();

		if (EditorSettings::Get().gridEnabled)
		{
			CU::Vector3f gridRotation;
			switch (EditorSettings::Get().gridPlane)
			{
			case GridPlane::X:
				gridRotation = { 0.0f, 0.0f, 90.0f };
				break;
			case GridPlane::Z:
				gridRotation = { 90.0f, 0.0f, 0.0f };
				break;
			default:
				break;
			}

			myDebugRenderer->DrawGrid(EditorSettings::Get().gridOffset, gridRotation, EditorSettings::Get().gridSize, EditorSettings::Get().gridOpacity);
		}

		if (SelectionManager::GetSelectionCount(SelectionContext::Scene) > 0)
		{
			for (UUID entityID : SelectionManager::GetSelections(SelectionContext::Scene))
			{
				Entity entity = myActiveScene->GetEntityWithUUID(entityID);

				if (entity.HasComponent<CameraComponent>())
				{
					const CU::Vector2f viewportSize = myGameViewport->MaxBounds() - myGameViewport->MinBounds();
					auto& cc = entity.GetComponent<CameraComponent>();
					cc.camera.SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);

					const CU::Matrix4x4f transform = myActiveScene->GetWorldSpaceTransformMatrix(entity);

					myDebugRenderer->DrawFrustum(transform.GetFastInverse(), cc.camera.GetProjectionMatrix(), CU::Color::Green);
				}

				if (entity.HasComponent<DirectionalLightComponent>())
				{
					auto& dlc = entity.GetComponent<DirectionalLightComponent>();

					const CU::Transform transform = myActiveScene->GetWorldSpaceTransform(entity);
					myDebugRenderer->DrawWireCone(transform.GetTranslation(), transform.GetRotation(), 5 * CU::Math::ToRad, 25, dlc.color);
				}

				if (entity.HasComponent<SpotlightComponent>())
				{
					auto& slc = entity.GetComponent<SpotlightComponent>();

					const CU::Transform transform = myActiveScene->GetWorldSpaceTransform(entity);
					myDebugRenderer->DrawWireCone(transform.GetTranslation(), transform.GetRotation(), slc.innerSpotAngle, slc.range, slc.color);
					myDebugRenderer->DrawWireCone(transform.GetTranslation(), transform.GetRotation(), slc.outerSpotAngle, slc.range, slc.color);
				}

				if (entity.HasComponent<PointLightComponent>())
				{
					auto& plc = entity.GetComponent<PointLightComponent>();

					const CU::Transform transform = myActiveScene->GetWorldSpaceTransform(entity);
					myDebugRenderer->DrawWireSphere(transform.GetTranslation(), transform.GetRotation(), plc.range, plc.color);
				}
			}
		}

		if (myShowCollidersMode != DebugLinesDrawMode::Off)
		{
			EPOCH_PROFILE_SCOPE("EditorLayer::OnRenderOverlay::Colliders");

			auto entities = myActiveScene->GetAllEntitiesWith<TransformComponent>();
			for (auto entityID : entities)
			{
				Entity entity = { entityID, myActiveScene.get() };
				if (!entity.IsActive() || myActiveScene->WasEntityFrustumCulled(entity.GetUUID())) continue;

				if (myShowCollidersMode == DebugLinesDrawMode::Selected)
				{
					if (!SelectionManager::IsSelected(SelectionContext::Scene, entity.GetUUID()))
					{
						continue;
					}

					if (entity.HasComponent<RigidbodyComponent>() && !entity.HasAny<BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent>())
					{
						for (auto child : entity.GetChildren())
						{
							OnRenderColliders(child);
						}
					}
					else
					{
						OnRenderColliders(entity);
					}
				}
				else
				{
					OnRenderColliders(entity);
				}
			}
		}

		if (myShowBoundingBoxesMode != DebugLinesDrawMode::Off)
		{
			EPOCH_PROFILE_SCOPE("EditorLayer::OnRenderOverlay::BoundingBoxes");

			auto meshEntities = myActiveScene->GetAllEntitiesWith<MeshRendererComponent>();
			for (auto entityID : meshEntities)
			{
				Entity entity = { entityID, myActiveScene.get() };
				auto& mrc = entity.GetComponent<MeshRendererComponent>();
				if (!entity.IsActive() || myActiveScene->WasEntityFrustumCulled(entity.GetUUID()) || !mrc.isActive) continue;

				if (myShowBoundingBoxesMode == DebugLinesDrawMode::Selected)
				{
					if (!SelectionManager::IsSelected(SelectionContext::Scene, entity.GetUUID()))
					{
						continue;
					}

					auto transform = myActiveScene->GetWorldSpaceTransformMatrix(entity);
					auto mesh = AssetManager::GetAssetAsync<Mesh>(mrc.mesh);
					if (mesh)
					{
						const AABB& aabb = mesh->GetBoundingBox();
						myDebugRenderer->DrawWireAABB(aabb, transform, CU::Color::Green);
					}
				}
				else
				{
					auto transform = myActiveScene->GetWorldSpaceTransformMatrix(entity);
					auto mesh = AssetManager::GetAssetAsync<Mesh>(mrc.mesh);
					if (mesh)
					{
						const AABB& aabb = mesh->GetBoundingBox();
						myDebugRenderer->DrawWireAABB(aabb, transform, CU::Color::Green);
					}
				}
			}
		}

		if (myShowGizmos)
		{
			// Camera Components
			{
					auto view = myActiveScene->GetAllEntitiesWith<CameraComponent>();
					for (auto entityID : view)
					{
						Entity entity{ entityID, myActiveScene.get() };
						const CU::Matrix4x4f transform = CU::Transform(entity.GetWorldSpaceTransform().GetTranslation(), myEditorCamera.GetTransform().GetRotation(), CU::Vector3f(myGizmoScale)).GetMatrix();
						myDebugRenderer->DrawQuad(EditorResources::CameraIcon, transform, CU::Color::White, (uint32_t)entity);
					}
				}

			// Sky Light Components
			{
					auto view = myActiveScene->GetAllEntitiesWith<SkyLightComponent>();
					for (auto entityID : view)
					{
						Entity entity{ entityID, myActiveScene.get() };
						const CU::Matrix4x4f transform = CU::Transform(entity.GetWorldSpaceTransform().GetTranslation(), myEditorCamera.GetTransform().GetRotation(), CU::Vector3f(myGizmoScale)).GetMatrix();
						myDebugRenderer->DrawQuad(EditorResources::SkyLightIcon, transform, CU::Color::White, (uint32_t)entity);
					}
				}

			// Directional Light Components
			{
					auto view = myActiveScene->GetAllEntitiesWith<DirectionalLightComponent>();
					for (auto entityID : view)
					{
						Entity entity{ entityID, myActiveScene.get() };
						const auto& ls = entity.GetComponent<DirectionalLightComponent>();
						const CU::Matrix4x4f transform = CU::Transform(entity.GetWorldSpaceTransform().GetTranslation(), myEditorCamera.GetTransform().GetRotation(), CU::Vector3f(myGizmoScale)).GetMatrix();
						myDebugRenderer->DrawQuad(EditorResources::DirectionalLightIcon, transform, ls.color, (uint32_t)entity);
					}
				}

			// Spotlight Components
			{
					auto view = myActiveScene->GetAllEntitiesWith<SpotlightComponent>();
					for (auto entityID : view)
					{
						Entity entity{ entityID, myActiveScene.get() };
						const auto& ls = entity.GetComponent<SpotlightComponent>();
						const CU::Matrix4x4f transform = CU::Transform(entity.GetWorldSpaceTransform().GetTranslation(), myEditorCamera.GetTransform().GetRotation(), CU::Vector3f(myGizmoScale)).GetMatrix();
						myDebugRenderer->DrawQuad(EditorResources::SpotlightIcon, transform, ls.color, (uint32_t)entity);
					}
				}

			// Point Light Components
			{
					auto view = myActiveScene->GetAllEntitiesWith<PointLightComponent>();
					for (auto entityID : view)
					{
						Entity entity{ entityID, myActiveScene.get() };
						const auto& ls = entity.GetComponent<PointLightComponent>();
						const CU::Matrix4x4f transform = CU::Transform(entity.GetWorldSpaceTransform().GetTranslation(), myEditorCamera.GetTransform().GetRotation(), CU::Vector3f(myGizmoScale)).GetMatrix();

						myDebugRenderer->DrawQuad(EditorResources::PointLightIcon, transform, ls.color, (uint32_t)entity);
					}
				}

			// ParticleEmitter Components
			{
					auto view = myActiveScene->GetAllEntitiesWith<ParticleSystemComponent>();
					for (auto entityID : view)
					{
						Entity entity{ entityID, myActiveScene.get() };
					}
				}
		}

		if (myDisplayUIInSceneView)
		{
			const float width = (float)myGameViewport->Size().x;
			const float height = (float)myGameViewport->Size().y;
			const CU::Vector3f bl = CU::Vector3f(0.0f, 0.0f, 0.0f);
			const CU::Vector3f br = CU::Vector3f(0.0f, height, 0.0f);
			const CU::Vector3f tl = CU::Vector3f(width, 0.0f, 0.0f);
			const CU::Vector3f tr = CU::Vector3f(width, height, 0.0f);

			myDebugRenderer->DrawRect(bl, br, tl, tr);

			if (myDisplayUIDebugRectsInSceneView)
			{
				auto view = myActiveScene->GetAllEntitiesWith<RectComponent>();
				for (auto id : view)
				{
					Entity entity{ id, myActiveScene.get() };
					if (!entity.IsActive()) continue;

					const auto& rc= view.get<RectComponent>(id);
					const CU::Matrix4x4f transform = entity.GetWorldSpaceTransform().GetMatrix();

					const CU::Vector2f size = CU::Vector2f((float)rc.size.x, (float)rc.size.y);
					const CU::Vector2f bl = (CU::Vector2f(0.0f, 0.0f) - rc.pivot) * size;
					const CU::Vector2f br = (CU::Vector2f(1.0f, 0.0f) - rc.pivot) * size;
					const CU::Vector2f tl = (CU::Vector2f(0.0f, 1.0f) - rc.pivot) * size;
					const CU::Vector2f tr = (CU::Vector2f(1.0f, 1.0f) - rc.pivot) * size;
					const CU::Vector4f bl4D = transform * CU::Vector4f(bl.x, bl.y, 0.0f, 1.0f);
					const CU::Vector4f br4D = transform * CU::Vector4f(br.x, br.y, 0.0f, 1.0f);
					const CU::Vector4f tl4D = transform * CU::Vector4f(tl.x, tl.y, 0.0f, 1.0f);
					const CU::Vector4f tr4D = transform * CU::Vector4f(tr.x, tr.y, 0.0f, 1.0f);
					myDebugRenderer->DrawRect(bl4D, br4D, tl4D, tr4D);
				}
			}
		}

		myDebugRenderer->Render(myEditorCamera.GetViewMatrix(), myEditorCamera.GetProjectionMatrix(), myDebugRendererOnTop);
	}

	void EditorLayer::OnRenderColliders(Entity aEntity)
	{
		if (aEntity.HasComponent<BoxColliderComponent>())
		{
			auto& bcc = aEntity.GetComponent<BoxColliderComponent>();

			const CU::Transform transform = myActiveScene->GetWorldSpaceTransform(aEntity);

			const CU::Vector3f pos = transform.GetTranslation() + (transform.GetRotationQuat().GetRotationMatrix3x3() * bcc.offset);
			myDebugRenderer->DrawWireBox(pos, transform.GetRotation(), bcc.halfSize * transform.GetScale(), CU::Color::Green);
		}

		if (aEntity.HasComponent<SphereColliderComponent>())
		{
			auto& scc = aEntity.GetComponent<SphereColliderComponent>();

			const CU::Transform transform = myActiveScene->GetWorldSpaceTransform(aEntity);

			const CU::Vector3f pos = transform.GetTranslation() + (transform.GetRotationQuat().GetRotationMatrix3x3() * scc.offset);
			float radius = CU::Math::Max(transform.GetScale().x, CU::Math::Max(transform.GetScale().y, transform.GetScale().z)) * scc.radius;
			myDebugRenderer->DrawWireSphere(pos, transform.GetRotation(), radius, CU::Color::Green);
		}

		if (aEntity.HasAny<CapsuleColliderComponent, CharacterControllerComponent>())
		{
			CU::Vector3f pos;
			float radius = 0;
			float height = 0;

			const CU::Transform transform = myActiveScene->GetWorldSpaceTransform(aEntity);

			if (aEntity.HasComponent<CapsuleColliderComponent>())
			{
				auto& ccc = aEntity.GetComponent<CapsuleColliderComponent>();
				pos = transform.GetTranslation() + (transform.GetRotationQuat().GetRotationMatrix3x3() * ccc.offset);
				radius = CU::Math::Max(transform.GetScale().x, transform.GetScale().z) * ccc.radius;
				height = ccc.height * transform.GetScale().y;
			}
			else
			{
				auto& ccc = aEntity.GetComponent<CharacterControllerComponent>();
				pos = transform.GetTranslation() + (transform.GetRotationQuat().GetRotationMatrix3x3() * ccc.offset);
				radius = CU::Math::Max(transform.GetScale().x, transform.GetScale().z) * ccc.radius;
				height = ccc.height * transform.GetScale().y;
			}

			myDebugRenderer->DrawWireCapsule(pos, transform.GetRotation(), radius, height, CU::Color::Green);
		}
	}

	ImGuizmo::OPERATION EditorLayer::GetGizmoOperation()
	{
		switch (myGizmoOperation)
		{
		case Epoch::EditorLayer::GizmoOperation::Translate:	return ImGuizmo::OPERATION::TRANSLATE;
		case Epoch::EditorLayer::GizmoOperation::Rotate:	return ImGuizmo::OPERATION::ROTATE;
		case Epoch::EditorLayer::GizmoOperation::Scale:		return ImGuizmo::OPERATION::SCALE;
		default: return ImGuizmo::OPERATION::TRANSLATE;
		}
	}

	void EditorLayer::DrawGizmos()
	{
		EPOCH_PROFILE_FUNC();

		if (myGizmoOperation == GizmoOperation::None) return;

		const auto& selections = SelectionManager::GetSelections(SelectionContext::Scene);
		if (selections.empty()) return;

		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

		const bool snap = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);

		const float snapValue = GetGizmoSnapValue();
		const float snapValues[3] = { snapValue, snapValue, snapValue };

		CU::Matrix4x4f projectionMatrix = myEditorCamera.GetProjectionMatrix();
		CU::Matrix4x4f viewMatrix = myEditorCamera.GetViewMatrix();

		ImGuizmo::OPERATION operation = GetGizmoOperation();
		ImGuizmo::MODE mode = (ImGuizmo::MODE)EditorSettings::Get().axisOrientationMode;
		
		bool cute = true;
		if (cute)
		{
			LOG_DEBUG("cute uwu :>");
		}

		if (selections.size() == 1)
		{
			Entity entity = myActiveScene->GetEntityWithUUID(SelectionManager::GetSelections(SelectionContext::Scene)[0]);
			CU::Transform& entityTransform = entity.Transform();
			CU::Matrix4x4f transform = myActiveScene->GetWorldSpaceTransformMatrix(entity);

			if (ImGuizmo::Manipulate(
				&viewMatrix[0],
				&projectionMatrix[0],
				operation,
				mode,
				&transform[0],
				nullptr,
				snap ? snapValues : nullptr))
			{
				Entity parent = myActiveScene->TryGetEntityWithUUID(entity.GetParentUUID());
				if (parent)
				{
					CU::Matrix4x4f parentTransform = myActiveScene->GetWorldSpaceTransformMatrix(parent);
					transform = transform * parentTransform.GetFastInverse();
				}

				CU::Vector3f translation, rotation, scale;
				transform.Decompose(translation, rotation, scale);

				switch (myGizmoOperation)
				{
				case EditorLayer::GizmoOperation::Translate:
				{
					const CU::Vector3f originalTranslation = entityTransform.GetTranslation();
					CU::Vector3f deltaTranslation = translation - originalTranslation;

					if (fabs(deltaTranslation.x) < CU::Math::Epsilon) deltaTranslation.x = 0.0f;
					if (fabs(deltaTranslation.y) < CU::Math::Epsilon) deltaTranslation.y = 0.0f;
					if (fabs(deltaTranslation.z) < CU::Math::Epsilon) deltaTranslation.z = 0.0f;

					entityTransform.Translate(deltaTranslation);
					break;
				}
				case EditorLayer::GizmoOperation::Rotate:
				{
					CU::Vector3f originalRotation = entityTransform.GetRotation();

					originalRotation.x = fmodf(originalRotation.x + CU::Math::Pi, CU::Math::Tau) - CU::Math::Pi;
					originalRotation.y = fmodf(originalRotation.y + CU::Math::Pi, CU::Math::Tau) - CU::Math::Pi;
					originalRotation.z = fmodf(originalRotation.z + CU::Math::Pi, CU::Math::Tau) - CU::Math::Pi;

					CU::Vector3f deltaRotation = rotation - originalRotation;

					if (fabs(deltaRotation.x) < CU::Math::Epsilon) deltaRotation.x = 0.0f;
					if (fabs(deltaRotation.y) < CU::Math::Epsilon) deltaRotation.y = 0.0f;
					if (fabs(deltaRotation.z) < CU::Math::Epsilon) deltaRotation.z = 0.0f;

					entityTransform.Rotate(deltaRotation);
					break;
				}
				case EditorLayer::GizmoOperation::Scale:
				{
					const CU::Vector3f originalScale = entityTransform.GetScale();
					CU::Vector3f deltaScale = scale - originalScale;

					if (fabs(deltaScale.x) < CU::Math::Epsilon) deltaScale.x = 0.0f;
					if (fabs(deltaScale.y) < CU::Math::Epsilon) deltaScale.y = 0.0f;
					if (fabs(deltaScale.z) < CU::Math::Epsilon) deltaScale.z = 0.0f;

					entityTransform.Scale(deltaScale);
					break;
				}
				}
			}
		}
		else
		{
			CU::Vector3f medianTranslation;
			CU::Vector3f medianRotation;
			CU::Vector3f medianScale;
			for (UUID entityID : selections)
			{
				Entity entity = myActiveScene->GetEntityWithUUID(entityID);
				myActiveScene->ConvertToWorldSpace(entity); // Needed for the matrix to be relative, but fucks up if parent and child selected
				const CU::Transform& entityTransform = entity.Transform();
				medianTranslation += entityTransform.GetTranslation();
				medianRotation += entityTransform.GetRotation();
				medianScale += entityTransform.GetScale();
			}
			medianTranslation /= (float)selections.size();
			medianRotation /= (float)selections.size();
			medianScale /= (float)selections.size();

			CU::Matrix4x4f medianMatrix = CU::Transform(medianTranslation, medianRotation, medianScale).GetMatrix();
			CU::Matrix4x4f deltaMatrix = CU::Matrix4x4f::Identity;

			if (ImGuizmo::Manipulate(
				&viewMatrix[0],
				&projectionMatrix[0],
				operation,
				mode,
				&medianMatrix[0],
				&deltaMatrix[0],
				snap ? snapValues : nullptr))
			{
				switch (EditorSettings::Get().multiTransformTarget)
				{
				case TransformationTarget::MedianPoint:
				{
					for (auto entityID : selections)
					{
						Entity entity = myActiveScene->GetEntityWithUUID(entityID);
						TransformComponent& transform = entity.GetComponent<TransformComponent>();
						transform.transform.SetTransform(transform.transform.GetMatrix() * deltaMatrix);
					}

					break;
				}
				case TransformationTarget::IndividualOrigins:
				{
					CU::Vector3f deltaTranslation, deltaRotation, deltaScale;
					deltaMatrix.Decompose(deltaTranslation, deltaRotation, deltaScale);

					for (auto entityID : selections)
					{
						Entity entity = myActiveScene->GetEntityWithUUID(entityID);
						CU::Transform& transform = entity.Transform();

						switch (myGizmoOperation)
						{
						case EditorLayer::GizmoOperation::Translate:
						{
							if (fabs(deltaTranslation.x) < CU::Math::Epsilon) deltaTranslation.x = 0.0f;
							if (fabs(deltaTranslation.y) < CU::Math::Epsilon) deltaTranslation.y = 0.0f;
							if (fabs(deltaTranslation.z) < CU::Math::Epsilon) deltaTranslation.z = 0.0f;

							transform.Translate(deltaTranslation);
							break;
						}
						case EditorLayer::GizmoOperation::Rotate:
						{
							if (fabs(deltaRotation.x) < CU::Math::Epsilon) deltaRotation.x = 0.0f;
							if (fabs(deltaRotation.y) < CU::Math::Epsilon) deltaRotation.y = 0.0f;
							if (fabs(deltaRotation.z) < CU::Math::Epsilon) deltaRotation.z = 0.0f;

							transform.Rotate(deltaRotation);
							break;
						}
						case EditorLayer::GizmoOperation::Scale:
						{
							if (fabs(deltaScale.x) < CU::Math::Epsilon) deltaScale.x = 0.0f;
							if (fabs(deltaScale.y) < CU::Math::Epsilon) deltaScale.y = 0.0f;
							if (fabs(deltaScale.z) < CU::Math::Epsilon) deltaScale.z = 0.0f;

							const CU::Vector3f originalScale = transform.GetScale();
							transform.Scale(deltaScale);
							break;
						}
						}
					}

					break;
				}
				default:
					break;
				}
			}

			// Needed for the matrix to be relative, but fucks up if parent and child selected
			for (UUID entityID : selections)
			{
				Entity entity = myActiveScene->GetEntityWithUUID(entityID);
				myActiveScene->ConvertToLocalSpace(entity);
			}
		}
	}

	float EditorLayer::GetGizmoSnapValue()
	{
		const auto& editorSettings = EditorSettings::Get();

		switch (myGizmoOperation)
		{
		case GizmoOperation::Translate: return editorSettings.translationSnapValue;
		case GizmoOperation::Rotate: return editorSettings.rotationSnapValue;
		case GizmoOperation::Scale: return editorSettings.scaleSnapValue;
		}

		return 0.0f;
	}

	void EditorLayer::CreateProject(const std::filesystem::path& aPath)
	{
		EPOCH_PROFILE_FUNC();

		if (!std::filesystem::exists(aPath))
		{
			std::filesystem::create_directories(aPath);
		}

		std::filesystem::copy("Resources/NewProjectTemplate", aPath, std::filesystem::copy_options::recursive);

		std::string projName = aPath.stem().string();

		// Premake File
		{
			std::ifstream stream(aPath / "premake5.lua");
			EPOCH_ASSERT(stream.is_open(), "Could not open projects premake file!");
			std::stringstream ss;
			ss << stream.rdbuf();
			stream.close();

			std::string str = ss.str();
			CU::ReplaceToken(str, "$PROJECT_NAME$", projName);

			std::ofstream ostream(aPath / "premake5.lua");
			ostream << str;
			ostream.close();
		}

		// Project File
		{
			std::ifstream stream(aPath / "Project.eproj");
			EPOCH_ASSERT(stream.is_open(), "Could not open project file!");
			std::stringstream ss;
			ss << stream.rdbuf();
			stream.close();

			std::string str = ss.str();
			CU::ReplaceToken(str, "$PROJECT_NAME$", projName);

			std::ofstream ostream(aPath / "Project.eproj");
			ostream << str;
			ostream.close();

			std::string newProjectFileName = std::string(projName) + ".eproj";
			std::filesystem::rename(aPath / "Project.eproj", aPath / newProjectFileName);
		}

		std::filesystem::create_directory(aPath / "Assets");
		std::filesystem::create_directory(aPath / "Regs");

		ScriptBuilder::RegenerateScriptSolution(aPath.string());
		OpenProject(aPath / (projName + ".eproj"));
	}

	void EditorLayer::SaveProject()
	{
		if (!Project::GetActive()) return;

		auto project = Project::GetActive();

		auto& userConf = project->GetUserConfig();

		userConf.startScene = "";
		if (!myEditorScenePath.empty())
		{
			project->GetUserConfig().startScene = std::filesystem::relative(myEditorScenePath, Project::GetAssetDirectory());
		}

		userConf.editorCameraPosition = myEditorCamera.GetTransform().GetTranslation();
		userConf.editorCameraRotation = myEditorCamera.GetTransform().GetRotation();

		ProjectSerializer serializer(project);
		serializer.Serialize(Project::GetProjectPath());
	}

	void EditorLayer::CloseProject()
	{
		SaveProject();

		SelectionManager::DeselectAll();
		ScriptEngine::SetSceneContext(nullptr, nullptr);

		myPanelManager->OnSceneChanged(nullptr);
		myActiveScene = nullptr;

		EPOCH_ASSERT(myEditorScene.use_count() == 1, "Scene will not be destroyed after project is closed - something is still holding scene refs!");
		myEditorScene = nullptr;

		Project::SetActive(nullptr);
	}

	bool EditorLayer::OpenProject()
	{
		std::filesystem::path filepath = FileSystem::OpenFileDialog({ { "Epoch project", "eproj" } });

		if (filepath.empty())
		{
			return false;
		}

		OpenProject(filepath);
		return true;
	}

	void EditorLayer::OpenProject(const std::filesystem::path& aPath)
	{
		EPOCH_PROFILE_FUNC();

		if (!std::filesystem::exists(aPath))
		{
			LOG_ERROR("Tried to open a project that doesn't exist. Project path: {}", aPath.string());
			return;
		}

		if (Project::GetActive())
		{
			CloseProject();
		}

		std::shared_ptr<Project> project = std::make_shared<Project>();
		ProjectSerializer serializer(project);
		serializer.Deserialize(aPath);
		Project::SetActive(project);

		auto appAssemblyPath = Project::GetScriptModuleFilePath();
		if (!appAssemblyPath.empty())
		{
			ScriptBuilder::RegenerateScriptSolution(Project::GetProjectDirectory());
			ScriptBuilder::BuildScriptAssembly();

			ScriptEngine::OnProjectChanged(project);
			ScriptEngine::LoadAppAssembly();
		}

		std::filesystem::path assetDirectory = Project::GetAssetDirectory();
		if (!std::filesystem::exists(assetDirectory)) FileSystem::CreateDirectory(assetDirectory);

		myPanelManager->OnProjectChanged(project);

		const auto& userConf = project->GetUserConfig();

		bool hasStartScene = !userConf.startScene.empty();
		const auto& startScenePath = assetDirectory / userConf.startScene;
		if (hasStartScene && FileSystem::Exists(startScenePath))
		{
			if (!OpenScene(startScenePath.string()))
			{
				NewScene();
			}
		}
		else
		{
			NewScene();
		}

		myEditorCamera.GetTransform().SetTranslation(userConf.editorCameraPosition);
		myEditorCamera.GetTransform().SetRotation(userConf.editorCameraRotation);

		RecentProject projectEntry;
		projectEntry.name = aPath.stem().string();
		projectEntry.filePath = Project::GetProjectPath().string();
		projectEntry.lastOpened = time(NULL);

		for (auto it = EditorSettings::Get().recentProjects.begin(); it != EditorSettings::Get().recentProjects.end(); it++)
		{
			if (it->second.filePath == projectEntry.filePath)
			{
				EditorSettings::Get().recentProjects.erase(it);
				break;
			}
		}

		EditorSettings::Get().recentProjects[projectEntry.lastOpened] = projectEntry;

		EditorSettings::Get().lastProjectPath = Project::GetProjectPath().string();
		EditorSettingsSerializer::SaveSettings();
	}

	void EditorLayer::UpdateWindowTitle(const std::string& aSceneName)
	{
		const std::string title = std::format("{} - {} - Epoch <{}>", aSceneName, Project::GetActive()->GetConfig().name, "DX11");
		Application::Get().GetWindow().SetTitle(title);
	}

	void EditorLayer::OpenScene()
	{
		std::filesystem::path filepath = FileSystem::OpenFileDialog({ { "Epoch Scene", "epoch" } }, Project::GetAssetDirectory().string().c_str());

		if (!filepath.empty())
		{
			OpenScene(filepath);
		}
	}

	bool EditorLayer::OpenScene(const std::filesystem::path& aPath, const bool aCheckAutoSave)
	{
		EPOCH_PROFILE_FUNC();

		if (!std::filesystem::exists(aPath))
		{
			LOG_ERROR("Tried loading a non-existing scene: {}", aPath.string());
			return false;
		}

		if (aPath.extension().string() != ".epoch")
		{
			LOG_ERROR("Could not load {} - not a scene file.", aPath.filename().string());
			return false;
		}


		std::filesystem::path scenePath = aPath;
		if (aCheckAutoSave)
		{
			std::string autoSavePath = (Project::GetAutosaveDirectory() / aPath.stem()).string() + ".auto";
			if (FileSystem::Exists(autoSavePath) && FileSystem::IsNewer(autoSavePath, aPath))
			{
				scenePath = std::filesystem::path(autoSavePath);
			}
		}

		auto assetManager = Project::GetEditorAssetManager();
		const auto& metadata = assetManager->GetMetadata(assetManager->GetAssetHandleFromFilePath(aPath));

		std::shared_ptr<Scene> newScene = std::make_shared<Scene>(metadata.handle);
		SceneSerializer serializer(newScene);

		if (!serializer.Deserialize(scenePath))
		{
			return false;
		}

		if (mySceneState != SceneState::Edit)
		{
			OnSceneStop();
		}

		SelectionManager::DeselectAll(SelectionContext::Scene);
		ScriptEngine::SetSceneContext(nullptr, nullptr);

		UpdateWindowTitle(aPath.stem().string());

		myEditorScene = newScene;
		myEditorScenePath = aPath;
		
		auto sceneRenderer = mySceneViewport->GetSceneRenderer();
		myActiveScene = myEditorScene;
		myPanelManager->OnSceneChanged(myActiveScene);
		if (sceneRenderer) sceneRenderer->SetScene(nullptr);
		ScriptEngine::SetSceneContext(myActiveScene, sceneRenderer);

		myTimeSinceLastSave = 0.0f;

		auto& userConf = Project::GetActive()->GetUserConfig();

		if (!myEditorScenePath.empty())
		{
			userConf.startScene = std::filesystem::relative(myEditorScenePath, Project::GetAssetDirectory());
		}

		RecentScene sceneEntry;
		sceneEntry.name = aPath.stem().string();
		sceneEntry.filePath = aPath.string();
		sceneEntry.lastOpened = time(NULL);

		for (auto it = userConf.recentScenes.begin(); it != userConf.recentScenes.end(); it++)
		{
			if (it->second.filePath == sceneEntry.filePath)
			{
				userConf.recentScenes.erase(it);
				break;
			}
		}

		userConf.recentScenes[sceneEntry.lastOpened] = sceneEntry;

		return true;
	}

	void EditorLayer::NewScene(const std::string& aName)
	{
		SelectionManager::DeselectAll(SelectionContext::Scene);
		ScriptEngine::SetSceneContext(nullptr, nullptr);

		UpdateWindowTitle(aName);

		myEditorScene = std::make_shared<Scene>(aName);
		myEditorScenePath = std::filesystem::path();
		
		auto sceneRenderer = mySceneViewport->GetSceneRenderer();
		myActiveScene = myEditorScene;
		myPanelManager->OnSceneChanged(myActiveScene);
		ScriptEngine::SetSceneContext(myActiveScene, sceneRenderer);

		if (sceneRenderer) sceneRenderer->SetScene(nullptr);
	}

	void EditorLayer::SaveScene()
	{
		if (!myEditorScenePath.empty())
		{
			SerializeScene(myActiveScene, myEditorScenePath);
		}
		else
		{
			SaveSceneAs();
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		if (!myActiveScene) return;

		std::filesystem::path filepath = FileSystem::SaveFileDialog({ { "Epoch Scene", "epoch" } }, Project::GetAssetDirectory().string().c_str());

		if (filepath.empty()) return;

		SerializeScene(myActiveScene, filepath);
		myEditorScenePath = filepath;
		UpdateWindowTitle(filepath.stem().string());

		auto assetManager = Project::GetEditorAssetManager();
		const auto& assetHandle = assetManager->ImportAsset(filepath);
		myActiveScene->SetAssetHandle(assetHandle);
	}

	void EditorLayer::SaveSceneAuto()
	{
		if (!myEditorScenePath.empty())
		{
			std::string autoSavePath = (Project::GetAutosaveDirectory() / myEditorScenePath.stem()).string() + ".auto";
			SerializeScene(myEditorScene, autoSavePath);
			LOG_INFO("Scene autosaved: {}", std::filesystem::relative(autoSavePath, Project::GetProjectDirectory()).string());
		}
	}

	void EditorLayer::SerializeScene(std::shared_ptr<Scene> aScene, const std::filesystem::path& aPath)
	{
		SceneSerializer serializer(aScene);
		serializer.Serialize(aPath.string());
		myTimeSinceLastSave = 0.0f;
	}

	void EditorLayer::QueueSceneTransition(AssetHandle aScene)
	{
		myPostSceneUpdateQueue.emplace_back([this, aScene]() { OnSceneTransition(aScene); });
	}

	void EditorLayer::EditorOptionsPanel()
	{
		if (ImGui::Begin("Editor Options (TEMP)"))
		{
			UI::BeginPropertyGrid();

			//const char* projTypeStrings[] = { "Perspective", "Orthographic" };
			//int currentProjType = 0;
			//if (UI::Property_Dropdown("Projection", projTypeStrings, 2, currentProjType))
			//{
			//	//TODO: Change editor camera projection type
			//}

			auto sceneRenderer = mySceneViewport->GetSceneRenderer();
			const char* drawModeStrings[] = { "Shaded", "Albedo", "Normals", "Ambient Occlusion", "Roughness", "Metalness", "Emission" };
			uint32_t currentDrawMode = (int)sceneRenderer->GetDrawMode();
			if (UI::Property_Dropdown("Draw Mode", drawModeStrings, 7, currentDrawMode))
			{
				sceneRenderer->SetDrawMode((DrawMode)currentDrawMode);
			}

			UI::EndPropertyGrid();

			UI::Spacing();
			UI::Draw::Underline();
			UI::Spacing();

			UI::BeginPropertyGrid();

			UI::Property_Checkbox("Cull with Scene Camera", myCullWithSceneCamera, "The primary camera in the scene will be used for culling.");

			UI::EndPropertyGrid();

			UI::Spacing();
			UI::Draw::Underline();
			UI::Spacing();

			UI::BeginPropertyGrid();

			UI::Property_Checkbox("Lines on Top", myDebugRendererOnTop, "True if the debug render should draw the lines on top.");

			UI::EndPropertyGrid();

			UI::Spacing();

			UI::BeginPropertyGrid();

			const char* debugLinesDrawModeStrings[] = { "Off", "All", "Selected" };

			uint32_t currentShowBoundingBoxesMode = (int)myShowBoundingBoxesMode;
			if (UI::Property_Dropdown("Show Bounding Box(es)", debugLinesDrawModeStrings, 3, currentShowBoundingBoxesMode))
			{
				myShowBoundingBoxesMode = (DebugLinesDrawMode)currentShowBoundingBoxesMode;
			}

			uint32_t currentShowCollidersMode = (int)myShowCollidersMode;
			if (UI::Property_Dropdown("Show Collider(s)", debugLinesDrawModeStrings, 3, currentShowCollidersMode))
			{
				myShowCollidersMode = (DebugLinesDrawMode)currentShowCollidersMode;
			}

			UI::EndPropertyGrid();

			UI::Spacing();
			UI::Draw::Underline();
			UI::Spacing();

			UI::BeginPropertyGrid();

			UI::Property_Checkbox("Show Gizmos", myShowGizmos);
			UI::Property_DragFloat("Gizmos Scale", myGizmoScale, 0.02f, 0.2f, 1.0f);

			UI::EndPropertyGrid();

			UI::Spacing();
			UI::Draw::Underline();
			UI::Spacing();

			UI::BeginPropertyGrid();

			UI::Property_Checkbox("Display UI in Scene View", myDisplayUIInSceneView);
			UI::Property_Checkbox("Display UI Debug Rects in Scene View", myDisplayUIDebugRectsInSceneView);

			UI::EndPropertyGrid();

			UI::Spacing();
			UI::Draw::Underline();
			UI::Spacing();

			UI::BeginPropertyGrid();

			UI::Property_Checkbox("Post Processing in Scene View", myPostProcessingInSceneView);
			UI::Property_Checkbox("Show Color Grading LUT", myDisplayCurrentColorGradingLUT);

			UI::EndPropertyGrid();

			//UI::Spacing(10);
			//
			//
			//static float value = 0;
			//value += CU::Timer::GetDeltaTime();
			//value = CU::Math::Wrap(value, 0.0f, 1.0f);
			//
			//UI::Widgets::Spinner("Test", CU::Math::Min(10.0f, ImGui::GetContentRegionAvail().x), 4.0f, Colors::Theme::disabled);
			//UI::Widgets::BufferingBar("Test", value, { CU::Math::Min(200.0f, ImGui::GetContentRegionAvail().x), 5.0f }, Colors::Theme::disabled, Colors::Theme::blue);


			UI::Spacing(5);
			UI::Draw::Underline();
			UI::Spacing(5);


			static CU::Vector2f p1 = { 0.0f, 0.0f };
			static CU::Vector2f p2 = { 0.5f, 0.0f };
			static CU::Vector2f p3 = { 0.5f, 1.0f };
			static CU::Vector2f p4 = { 1.0f, 1.0f };

			UI::BeginPropertyGrid();

			UI::Property_DragFloat("P1", p1.y, 0.01f, 0.0f, 1.0f);
			UI::Property_DragFloat2("P2", p2, 0.01f, 0.0f, 1.0f);
			UI::Property_DragFloat2("P3", p3, 0.01f, 0.0f, 1.0f);
			UI::Property_DragFloat("P4", p4.y, 0.01f, 0.0f, 1.0f);

			UI::Property_CubicBezier("Test", p1, p2, p3, p4);

			UI::EndPropertyGrid();

			float barSize = ImGui::GetContentRegionAvail().x;
			ImVec2 aBarPos = ImGui::GetCursorScreenPos();
			UI::Widgets::CubicBezier({ p1, p2, p3, p4 }, { aBarPos.x, aBarPos.y }, barSize, barSize, true);

			//auto [mx, my] = ImGui::GetMousePos();
			//mx -= aBarPos.x;
			//my -= aBarPos.y;
			//my = barSize - my;
			//
			//CU::Vector2f mpos = { mx / barSize, my / barSize };
			//
			//ImGui::Text("%u x %u", (uint32_t)mx, (uint32_t)my);
			//ImGui::Text("%.3f x %.3f", mpos.x, mpos.y);

		}
		ImGui::End();
	}

	void EditorLayer::DisplayColorGradingLUT()
	{
		if (myDisplayCurrentColorGradingLUT)
		{
			AssetHandle lutHandle(0);
		
			auto entities = myActiveScene->GetAllEntitiesWith<VolumeComponent>();
			for (auto entityID : entities)
			{
				Entity entity = Entity(entityID, myActiveScene.get());
				if (!entity.IsActive()) continue;
		
				const auto& vc = entities.get<VolumeComponent>(entityID);
				if (!vc.isActive || !vc.colorGrading.enabled) continue;
		
				lutHandle = vc.colorGrading.lut;
		
				break;
			}
		
			ImGui::SetCursorScreenPos(mySceneViewport->MinBounds());
			UI::ShiftCursor(5, 5);
			if (mySceneViewport->GetSceneRenderer()->ColorGrading() && lutHandle != 0)
			{
				auto lut = AssetManager::GetAsset<Texture>(lutHandle);
				ImGui::Image((ImTextureID)lut->GetView(), { 256, 16 });
			}
			else
			{
				ImGui::Image((ImTextureID)Renderer::GetDefaultColorGradingLut()->GetView(), { 256, 16 });
			}
		}
	}

	void EditorLayer::ToolbarPanel()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colors = ImGui::GetStyle().Colors;
		const ImVec4& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

		if (ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse))
		{
			bool toolbarEnabled = (bool)myActiveScene;

			ImVec4 tintColor = ImVec4(1, 1, 1, 1);
			if (!toolbarEnabled)
			{
				tintColor.w = 0.5f;
			}

			float size = ImGui::GetWindowHeight() - 4.0f;
			ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));


			bool hasPlayButton = mySceneState == SceneState::Edit || mySceneState == SceneState::Play;
			bool hasSimulateButton = mySceneState == SceneState::Edit || mySceneState == SceneState::Simulate;
			bool hasPauseButton = mySceneState != SceneState::Edit;

			if (hasPlayButton)
			{
				auto icon = (mySceneState == SceneState::Edit) ? EditorResources::PlayButton : EditorResources::StopButton;
				if (ImGui::ImageButton((ImTextureID)icon->GetView(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
				{
					if (mySceneState == SceneState::Edit)
					{
						OnScenePlay();
					}
					else if (mySceneState == SceneState::Play)
					{
						OnSceneStop();
					}
				}
				if (ImGui::BeginItemTooltip())
				{
					(mySceneState == SceneState::Edit) ? ImGui::Text(" Play ") : ImGui::Text(" Stop ");
					ImGui::EndTooltip();
				}
			}

			if (hasSimulateButton)
			{
				if (hasPlayButton)
				{
					ImGui::SameLine();
				}

				auto icon = (mySceneState == SceneState::Edit) ? EditorResources::SimulateButton : EditorResources::StopButton;
				if (ImGui::ImageButton((ImTextureID)icon->GetView(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
				{
					if (mySceneState == SceneState::Edit)
					{
						OnSceneSimulate();
					}
					else if (mySceneState == SceneState::Simulate)
					{
						OnSceneStop();
					}

				}
				if (ImGui::BeginItemTooltip())
				{
					(mySceneState == SceneState::Edit) ? ImGui::Text(" Simulate ") : ImGui::Text(" Stop ");
					ImGui::EndTooltip();
				}
			}

			if (hasPauseButton)
			{
				bool isPaused = myActiveScene->IsPaused();
				ImGui::SameLine();
				{
					auto icon = isPaused ? EditorResources::PlayButton : EditorResources::PauseButton;
					if (ImGui::ImageButton((ImTextureID)icon->GetView(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
					{
						myActiveScene->SetPaused(!isPaused);
						if (!myActiveScene->IsPaused() && mySceneState == SceneState::Play)
						{
							myGameViewport->SetFocus();
						}
					}
					if (ImGui::BeginItemTooltip())
					{
						isPaused ? ImGui::Text(" Play ") : ImGui::Text(" Pause ");
						ImGui::EndTooltip();
					}
				}

				// Step button
				if (isPaused)
				{
					ImGui::SameLine();
					{
						auto icon = EditorResources::StepButton;
						ImGui::ImageButton((ImTextureID)icon->GetView(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled;

						static float stepHeldTime = 0.0f;
						if (ImGui::IsItemActive())
						{
							stepHeldTime -= CU::Timer::GetDeltaTime();
							if (stepHeldTime <= 0.0f)
							{
								stepHeldTime = 0.1f;
								myActiveScene->Step(5);
							}
						}

						if (ImGui::IsItemDeactivated())
						{
							stepHeldTime = 0.0f;
						}

						if (ImGui::BeginItemTooltip())
						{
							ImGui::Text("Step");
							ImGui::EndTooltip();
						}
					}
				}
			}
		}

		ImGui::End();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
	}

	void EditorLayer::OnScenePlay()
	{
		if (EditorSettings::Get().autoSaveSceneBeforePlay)
		{
			SaveSceneAuto();
		}

		mySceneState = SceneState::Play;

		myActiveScene = std::make_shared<Scene>();
		myEditorScene->CopyTo(myActiveScene);
		myActiveScene->SetSceneTransitionCallback([this](AssetHandle scene) { QueueSceneTransition(scene); });
		ScriptEngine::SetSceneContext(myActiveScene, mySceneViewport->GetSceneRenderer());

		myPanelManager->OnSceneChanged(myActiveScene);

		Application::Get().DispatchEvent<ScenePreStartEvent, true>(myActiveScene);
		myActiveScene->OnRuntimeStart();
		Application::Get().DispatchEvent<ScenePostStartEvent, true>(myActiveScene);

		myGameViewport->SetFocus();
		myStatisticsPanel->SetSceneRenderer(myGameViewport->GetSceneRenderer());
	}

	void EditorLayer::OnSceneSimulate()
	{
		if (EditorSettings::Get().autoSaveSceneBeforePlay)
		{
			SaveSceneAuto();
		}

		mySceneState = SceneState::Simulate;

		myActiveScene = std::make_shared<Scene>();
		myEditorScene->CopyTo(myActiveScene);

		myPanelManager->OnSceneChanged(myActiveScene);
		myActiveScene->OnSimulationStart();
	}

	void EditorLayer::OnSceneStop()
	{
		bool stoppedPlay = false;
		if (mySceneState == SceneState::Play)
		{
			Application::Get().DispatchEvent<ScenePreStopEvent, true>(myActiveScene);
			myActiveScene->OnRuntimeStop();
			Application::Get().DispatchEvent<ScenePostStopEvent, true>(myActiveScene);
			
			stoppedPlay = true;
		}
		else if (mySceneState == SceneState::Simulate)
		{
			myActiveScene->OnSimulationStop();
		}

		ScriptEngine::SetSceneContext(nullptr, nullptr);

		mySceneState = SceneState::Edit;
		myActiveScene = myEditorScene;

		auto selection = SelectionManager::GetSelections(SelectionContext::Scene);
		for (auto id : selection)
		{
			if (!myActiveScene->TryGetEntityWithUUID(id))
			{
				SelectionManager::Deselect(SelectionContext::Scene, id);
			}
		}

		ScriptEngine::SetSceneContext(myActiveScene, mySceneViewport->GetSceneRenderer());
		myPanelManager->OnSceneChanged(myActiveScene);

		Input::SetCursorMode(CursorMode::Normal);

		if (stoppedPlay)
		{
			mySceneViewport->SetFocus();
			myStatisticsPanel->SetSceneRenderer(mySceneViewport->GetSceneRenderer());
		}
	}

	void EditorLayer::OnSceneTransition(AssetHandle aScene)
	{
		const auto& metadata = Project::GetEditorAssetManager()->GetMetadata(aScene);

		std::shared_ptr<Scene> newScene = std::make_shared<Scene>(metadata.handle);
		SceneSerializer serializer(newScene);

		std::filesystem::path scenePath = metadata.filePath;
		if (serializer.Deserialize((Project::GetAssetDirectory() / scenePath).string()))
		{
			const CU::Vector2f viewportSize = mySceneViewport->MaxBounds() - mySceneViewport->MinBounds();
			newScene->SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);

			myActiveScene->OnRuntimeStop();

			SelectionManager::DeselectAll();

			auto sceneRenderer = mySceneViewport->GetSceneRenderer();
			myActiveScene = newScene;
			myActiveScene->SetSceneTransitionCallback([this](AssetHandle scene) { QueueSceneTransition(scene); });
			ScriptEngine::SetSceneContext(myActiveScene, sceneRenderer);
			myActiveScene->OnRuntimeStart();
			myPanelManager->OnSceneChanged(myActiveScene);
		}
		else
		{
			CONSOLE_LOG_ERROR("Could not deserialize scene {} {}", scenePath.string(), aScene);
		}
	}

	void EditorLayer::OnEntityCreated(Entity aEntity)
	{
		if (!EditorSettings::Get().createEntitiesAtOrigin)
		{
			const CU::Transform& cameraTransform = myEditorCamera.GetTransform();
			aEntity.Transform().SetTranslation(cameraTransform.GetTranslation() + cameraTransform.GetForward() * 500.0f);
		}
	}

	void EditorLayer::OnEntityDeleted(Entity aEntity)
	{
		SelectionManager::Deselect(SelectionContext::Scene, aEntity.GetUUID());
	}

	bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& aEvent)
	{
		if ((!Input::IsMouseButtonHeld(MouseButton::Right) && !Input::IsMouseButtonHeld(MouseButton::Middle)))
		{
			if (mySceneState != SceneState::Play)
			{
				if (mySceneState == SceneState::Edit && Input::IsKeyHeld(KeyCode::LeftControl))
				{
					switch (aEvent.GetKeyCode())
					{
					case KeyCode::O:
						OpenScene();
						break;
					case KeyCode::N:
						NewScene();
						break;
					case KeyCode::S:
					{
						if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
						{
							SaveSceneAs();
						}
						else
						{
							SaveScene();
						}
						break;
					}
					}
				}
			}

			if (mySceneViewport->IsHovered() && !ImGui::IsAnyItemActive())
			{
				switch (aEvent.GetKeyCode())
				{
				case KeyCode::Q:
					myGizmoOperation = GizmoOperation::None;
					break;
				case KeyCode::W:
					myGizmoOperation = GizmoOperation::Translate;
					break;
				case KeyCode::E:
					myGizmoOperation = GizmoOperation::Rotate;
					break;
				case KeyCode::R:
					myGizmoOperation = GizmoOperation::Scale;
					break;
				case KeyCode::F:
				{
					if (SelectionManager::GetSelectionCount(SelectionContext::Scene) == 1)
					{
						Entity entity = myActiveScene->GetEntityWithUUID(SelectionManager::GetSelections(SelectionContext::Scene)[0]);
						myEditorCamera.Focus(entity.GetWorldSpaceTransform().GetTranslation());
					}
					break;
				}
				case KeyCode::X:
				{
					const bool isLocal = EditorSettings::Get().axisOrientationMode == AxisOrientationMode::Local;
					EditorSettings::Get().axisOrientationMode = isLocal ? AxisOrientationMode::World : AxisOrientationMode::Local;
					break;
				}
				}
			}
		}

		if (UI::IsWindowFocused(SCENE_PANEL_ID) || UI::IsWindowFocused(SCENE_HIERARCHY_PANEL_ID))
		{
			switch (aEvent.GetKeyCode())
			{
			case KeyCode::Delete:
			{
				auto selection = SelectionManager::GetSelections(SelectionContext::Scene);

				for (auto entityID : selection)
				{
					auto entity = myActiveScene->TryGetEntityWithUUID(entityID);
					if (!entity) continue;
					myActiveScene->DestroyEntity(entity);
				}

				break;
			}
			case KeyCode::D:
			{
				if (!Input::IsMouseButtonHeld(MouseButton::Right) && !Input::IsMouseButtonHeld(MouseButton::Middle) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
				{
					auto selection = SelectionManager::GetSelections(SelectionContext::Scene);
					for (const auto& entityID : selection)
					{
						Entity entity = myActiveScene->GetEntityWithUUID(entityID);

						Entity duplicate = myActiveScene->DuplicateEntity(entity);
						SelectionManager::Deselect(SelectionContext::Scene, entity.GetUUID());
						SelectionManager::Select(SelectionContext::Scene, duplicate.GetUUID());
					}
				}

				break;
			}
			}
		}

		if (aEvent.GetKeyCode() == KeyCode::P && Input::IsKeyHeld(KeyCode::LeftAlt) && (mySceneState != SceneState::Edit || mySceneViewport->IsFocused()))
		{
			if (mySceneState == SceneState::Play)
			{
				OnSceneStop();
			}
			else
			{
				OnScenePlay();
			}
		}

		if (aEvent.GetKeyCode() == KeyCode::F9)
		{
			myDisplayCurrentColorGradingLUT = !myDisplayCurrentColorGradingLUT;
		}

		return false;
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& aEvent)
	{
		if (aEvent.GetMouseButton() == MouseButton::Left)
		{
			auto sceneRenderer = mySceneViewport->GetSceneRenderer();
			if (sceneRenderer->GetDrawMode() != DrawMode::Shaded ||
				!mySceneViewport->IsHovered() ||
				ImGui::IsAnyItemHovered() ||
				ImGuizmo::IsOver() ||
				!mySceneViewport->MouseInViewport() ||
				Input::IsKeyHeld(KeyCode::LeftShift))
			{
				return false;
			}

			OnViewportClickSelection();
		}

		return false;
	}

	bool EditorLayer::OnViewportClickSelection()
	{
		const bool ctrlDown = Input::IsKeyHeld(KeyCode::LeftControl);

		ImGui::ClearActiveID();

		Entity clickedEntity = GetHoveredEntity();
		
		if (!clickedEntity)
		{
			SelectionManager::DeselectAll(SelectionContext::Scene);
		}
		else
		{
			if (!myActiveScene->IsEntityValid(clickedEntity)) return false;

			if (!ctrlDown)
			{
				SelectionManager::DeselectAll(SelectionContext::Scene);
			}

			if (ctrlDown && SelectionManager::IsSelected(SelectionContext::Scene, clickedEntity.GetUUID()))
			{
				SelectionManager::Deselect(SelectionContext::Scene, clickedEntity.GetUUID());
			}
			else
			{
				SelectionManager::Select(SelectionContext::Scene, clickedEntity.GetUUID());
			}
		}

		return true;
	}

	void EditorLayer::UpdateViewportBoxSelection()
	{
		static bool dragging = false;
		static CU::Vector2f boxStartPos;
		static CU::Vector2f boxEndPos;
		static CU::Vector2f dragStartPos;
		static CU::Vector2f dragEndPos;

		if (!mySceneViewport->IsFocused() ||
			(!dragging && ImGuizmo::IsOver()) ||
			!mySceneViewport->MouseInViewport())
		{
			return;
		}

		const bool ctrlDown = Input::IsKeyHeld(KeyCode::LeftControl);
		const bool shiftDown = Input::IsKeyHeld(KeyCode::LeftShift);
		const bool shiftReleased = Input::IsKeyReleased(KeyCode::LeftShift);
		
		if (shiftDown && Input::IsMouseButtonPressed(MouseButton::Left))
		{
			dragging = true;
			ImGui::ClearActiveID();
		
			auto mousePos = ImGui::GetMousePos();
			boxStartPos.x = mousePos.x;
			boxStartPos.y = mousePos.y;
		
			auto [px, py] = mySceneViewport->GetMouseViewportCord();
			dragStartPos = CU::Vector2f(px, py);
		}
		else if ((shiftReleased && dragging) || (Input::IsMouseButtonReleased(MouseButton::Left) && dragging))
		{
			dragging = false;
		
			auto [px, py] = mySceneViewport->GetMouseViewportCord();
			dragEndPos = CU::Vector2f(px, py);
		
			const CU::Vector2f selectionBoxMin = { CU::Math::Min(dragStartPos.x, dragEndPos.x), CU::Math::Min(dragStartPos.y, dragEndPos.y) };
			const CU::Vector2f selectionBoxMax = { CU::Math::Max(dragStartPos.x, dragEndPos.x), CU::Math::Max(dragStartPos.y, dragEndPos.y) };
			
			const CU::Vector2f boxSize = selectionBoxMax - selectionBoxMin;
			if (boxSize.x > 0 && boxSize.y > 0)
			{
				auto sceneRenderer = mySceneViewport->GetSceneRenderer();
				auto IDBuffer = sceneRenderer->GetEntityIDTexture();
				auto pixelData = IDBuffer->ReadData((uint32_t)boxSize.x, (uint32_t)boxSize.y, (uint32_t)selectionBoxMin.x, (uint32_t)selectionBoxMin.y);
				if (pixelData)
				{
					std::unordered_set<uint32_t> ids;
					for (size_t i = 0; i < pixelData.size; i += 4)
					{
						uint32_t id = pixelData.Read<uint32_t>(i);
						if (id == 0) continue;
						ids.insert(id - 1);
					}
					pixelData.Release();
		
					if (!ctrlDown)
					{
						SelectionManager::DeselectAll(SelectionContext::Scene);
					}
		
					for (uint32_t id : ids)
					{
						Entity clickedEntity = Entity((entt::entity)id, myActiveScene.get());
						if (!clickedEntity || !myActiveScene->IsEntityValid(clickedEntity)) continue;
		
						if (ctrlDown && SelectionManager::IsSelected(SelectionContext::Scene, clickedEntity.GetUUID()))
						{
							SelectionManager::Deselect(SelectionContext::Scene, clickedEntity.GetUUID());
						}
						else
						{
							SelectionManager::Select(SelectionContext::Scene, clickedEntity.GetUUID());
						}
					}
				}
			}
		}
		
		if (dragging)
		{
			auto mousePos = ImGui::GetMousePos();
			boxEndPos.x = mousePos.x;
			boxEndPos.y = mousePos.y;
		
			const CU::Vector2f selectionBoxMin = { CU::Math::Min(boxStartPos.x, boxEndPos.x), CU::Math::Min(boxStartPos.y, boxEndPos.y) };
			const CU::Vector2f selectionBoxMax = { CU::Math::Max(boxStartPos.x, boxEndPos.x), CU::Math::Max(boxStartPos.y, boxEndPos.y) };
		
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
		
			draw_list->AddRectFilled(ImVec2(selectionBoxMin.x, selectionBoxMin.y), ImVec2(selectionBoxMax.x, selectionBoxMax.y), Colors::Theme::dragBoxFill);
			draw_list->AddRect(ImVec2(selectionBoxMin.x, selectionBoxMin.y), ImVec2(selectionBoxMax.x, selectionBoxMax.y), Colors::Theme::dragBoxFrame);
		}
	}

	void EditorLayer::OnSnapToEditorCamera(Entity aEntity)
	{
		myActiveScene->ConvertToWorldSpace(aEntity);
		aEntity.Transform().SetTranslation(myEditorCamera.GetTransform().GetTranslation());
		aEntity.Transform().SetRotation(myEditorCamera.GetTransform().GetRotationQuat());
		myActiveScene->ConvertToLocalSpace(aEntity);
	}

	void EditorLayer::OnResetBoneTransforms(Entity aEntity)
	{
		if (aEntity.HasComponent<SkinnedMeshRendererComponent>())
		{
			auto& smrc = aEntity.GetComponent<SkinnedMeshRendererComponent>();
			auto meshAssset = AssetManager::GetAsset<Mesh>(smrc.mesh);

			if (!meshAssset->HasSkeleton())
			{
				return;
			}

			auto skeleton = meshAssset->GetSkeleton();

			for (uint32_t i = 0; i < skeleton->GetNumBones(); i++)
			{
				const Skeleton::Bone& bone = skeleton->GetBone(i);
				EPOCH_ASSERT(i < smrc.boneEntityIds.size(), "Missing bone!");
				auto boneEntity = myActiveScene->GetEntityWithUUID(smrc.boneEntityIds[i]);

				auto& tc = boneEntity.GetComponent<TransformComponent>();
				tc.transform.SetTranslation(bone.position);
				tc.transform.SetRotation(bone.orientation.GetEulerAngles());
				tc.transform.SetScale(bone.scale);
			}
		}
	}

	void EditorLayer::OnCurrentSceneRenamed(const AssetMetadata& aMetadata)
	{
		myEditorScenePath = Project::GetEditorAssetManager()->GetFileSystemPath(aMetadata);
		myEditorScene->SetName(aMetadata.filePath.stem().string());
		UpdateWindowTitle(aMetadata.filePath.stem().string());
	}
}
