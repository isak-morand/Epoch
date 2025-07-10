#include "EditorLayer.h"
#include <EpochCore/Log.h>
#include <EpochCore/Profiler.h>
#include <EpochEngine/Engine.h>
#include <EpochProjects/Project.h>

#include <EpochCore/FileSystem.h>
#include <EpochCore/Input/Input.h>
#include <EpochRendering/IRenderer.h>
#include <EpochAssets/AssetImporter.h>
#include <EpochAssets/Assets/TextureAsset.h>
#include <EpochAssets/Assets/ModelAsset.h>
#include <EpochAssets/Assets/MeshAsset.h>
#include <EpochAssets/AssetManager.h>
#include <EpochScenes/Scene.h>

#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

namespace Epoch::Editor
{
	static std::shared_ptr<Assets::MeshAsset> staticChestMesh;
	static std::shared_ptr<Assets::MeshAsset> staticChestBottomMesh;
	static std::shared_ptr<Assets::MeshAsset> staticChestLidMesh;
	static std::shared_ptr<Assets::MeshAsset> staticRaccoonMesh;

	void EditorLayer::OnAttach()
	{
		TypedAssetHandle<Assets::TextureAsset> textureAssetHandle(1761087208084911085);
		Engine::GetInstance()->GetRendererInterface()->SetTexture(Assets::AssetManager::GetAsset<Assets::TextureAsset>(textureAssetHandle));

		TypedAssetHandle<Assets::ModelAsset> chest1Handle(17818076198096816287); //Merged
		TypedAssetHandle<Assets::ModelAsset> chest2Handle(2244813137981101981); //Separate
		TypedAssetHandle<Assets::ModelAsset> raccoonHandle(2826550511294781016);
		TypedAssetHandle<Assets::ModelAsset> cubeTestHandle(10955629889549739932);

		TypedAssetHandle<Assets::MeshAsset> chest(4615339590844839697);
		staticChestMesh = Assets::AssetManager::GetAsset<Assets::MeshAsset>(chest);

		TypedAssetHandle<Assets::MeshAsset> bottom(8771711513993537053);
		staticChestBottomMesh = Assets::AssetManager::GetAsset<Assets::MeshAsset>(bottom);

		TypedAssetHandle<Assets::MeshAsset> lid(8771710414481908842);
		staticChestLidMesh = Assets::AssetManager::GetAsset<Assets::MeshAsset>(lid);

		TypedAssetHandle<Assets::MeshAsset> raccoon(9950345307428580679);
		staticRaccoonMesh = Assets::AssetManager::GetAsset<Assets::MeshAsset>(raccoon);

		Engine::GetInstance()->GetRendererInterface()->SetMesh(staticChestMesh);

		myScene = std::make_shared<Scenes::Scene>();
		
		Scenes::Entity cubes = myScene->Instantiate(Assets::AssetManager::GetAsset<Assets::ModelAsset>(cubeTestHandle));
		myScene->InstantiateChild(Assets::AssetManager::GetAsset<Assets::ModelAsset>(chest2Handle), cubes);
		myScene->InstantiateChild(Assets::AssetManager::GetAsset<Assets::ModelAsset>(chest1Handle), cubes);
		myScene->Instantiate(Assets::AssetManager::GetAsset<Assets::ModelAsset>(raccoonHandle));

		myScene->PrintHierarchy();
	}

	void EditorLayer::OnDetach()
	{
	}

	void EditorLayer::OnUpdate()
	{
		EPOCH_PROFILE_FUNC();

		if (Input::IsKeyPressed(KeyCode::D1))
		{
			Engine::GetInstance()->GetRendererInterface()->SetMesh(staticChestMesh);
		}
		else if (Input::IsKeyPressed(KeyCode::D2))
		{
			Engine::GetInstance()->GetRendererInterface()->SetMesh(staticChestBottomMesh);
		}
		else if (Input::IsKeyPressed(KeyCode::D3))
		{
			Engine::GetInstance()->GetRendererInterface()->SetMesh(staticChestLidMesh);
		}
		else if (Input::IsKeyPressed(KeyCode::D4))
		{
			Engine::GetInstance()->GetRendererInterface()->SetMesh(staticRaccoonMesh);
		}
	}

	void EditorLayer::OnRenderImGui()
	{
		EPOCH_PROFILE_FUNC();
	}

	void EditorLayer::OnEvent(Event& aEvent)
	{
	}
}
