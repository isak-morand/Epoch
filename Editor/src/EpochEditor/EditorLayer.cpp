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
		Engine::Get()->GetRendererInterface()->SetTexture(Assets::AssetManager::GetAsset<Assets::TextureAsset>(textureAssetHandle));
		

		myScene = std::make_shared<Scenes::Scene>();

		TypedAssetHandle<Assets::ModelAsset> chest1Handle(17818076198096816287); //Merged
		TypedAssetHandle<Assets::ModelAsset> chest2Handle(2244813137981101981); //Separate
		TypedAssetHandle<Assets::ModelAsset> raccoonHandle(2826550511294781016);
		TypedAssetHandle<Assets::ModelAsset> cubeTestHandle(10955629889549739932);
		TypedAssetHandle<Assets::ModelAsset> superStefanHandle(13809844527519340069);
		
		Scenes::Entity cubes = myScene->Instantiate(Assets::AssetManager::GetAsset<Assets::ModelAsset>(cubeTestHandle));
		cubes.GetComponent<Scenes::TransformComponent>().LocalTransform.SetTranslation({ -250, 0, 0 });

		Scenes::Entity separateChest = myScene->Instantiate(Assets::AssetManager::GetAsset<Assets::ModelAsset>(chest2Handle));
		separateChest.GetComponent<Scenes::TransformComponent>().LocalTransform.SetTranslation({ 250, 0, 150 });
		Scenes::Entity lid{ separateChest.GetComponent<Scenes::ChildrenComponent>().Children[1], myScene.get() };
		lid.GetComponent<Scenes::TransformComponent>().LocalTransform.SetRotation(-123.f * CU::Math::ToRad, 0.f, 0.f);
		myLidEntity = lid.GetUUID();
		
		Scenes::Entity mergedChest = myScene->Instantiate(Assets::AssetManager::GetAsset<Assets::ModelAsset>(chest1Handle));
		mergedChest.GetComponent<Scenes::TransformComponent>().LocalTransform.SetTranslation({ 250, 0, -150 });
		
		Scenes::Entity raccoon = myScene->Instantiate(Assets::AssetManager::GetAsset<Assets::ModelAsset>(raccoonHandle));
		
		Scenes::Entity stefan = myScene->Instantiate(Assets::AssetManager::GetAsset<Assets::ModelAsset>(superStefanHandle));
		stefan.GetComponent<Scenes::TransformComponent>().LocalTransform.SetTranslation({ 0, 0, -150 });

		myScene->PrintHierarchy();
	}

	void EditorLayer::OnDetach()
	{
	}

	void EditorLayer::OnUpdate()
	{
		EPOCH_PROFILE_FUNC();

		static bool opening = false;
		static bool open = false;

		if (!open && !opening && Input::IsKeyPressed(KeyCode::Space))
		{
			opening = true;
		}

		if (opening)
		{
			Scenes::Entity lid = myScene->GetEntityWithUUID(myLidEntity);
			CU::Transform& trans = lid.GetComponent<Scenes::TransformComponent>().LocalTransform;
			trans.Rotate(CU::Vector3f::Right * 30.f * CU::Math::ToRad * Engine::Get()->GetDeltaTime());

			if (trans.GetRotation().x >= 0.f)
			{
				open = true;
				opening = false;
			}
		}

		auto view = myScene->GetAllEntitiesWith<Scenes::MeshRendererComponent>();
		for (auto id : view)
		{
			Scenes::Entity entity{ id, myScene.get() };
			const auto& mrc = view.get<Scenes::MeshRendererComponent>(id);

			Engine::Get()->GetRendererInterface()->SubmitMesh(Assets::AssetManager::GetAsset<Assets::MeshAsset>(mrc.Mesh), myScene->GetWorldSpaceTransformMatrix(entity));
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
