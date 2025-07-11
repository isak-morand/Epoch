#include "Scene.h"
#include <EpochCore/UUID.h>
#include <EpochAssets/Assets/ModelAsset.h>

namespace Epoch::Scenes
{
	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity(std::string_view aName)
	{
		return CreateChildEntity({}, aName);
	}

	Entity Scene::CreateChildEntity(Entity aParent, std::string_view aName)
	{
		auto entity = Entity{ myRegistry.create(), this };
		UUID uuid = UUID();

		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<NameComponent>(aName);
		entity.AddComponent<TransformComponent>();

		if (aParent)
		{
			ParentEntity(entity, aParent);
		}

		myIDToEntityMap.insert_or_assign(uuid, entity);
		myEntityToIDMap.insert_or_assign(entity, uuid);

		return entity;
	}

	Entity Scene::GetEntityWithUUID(UUID aUUID)
	{
		EPOCH_ASSERT(myIDToEntityMap.contains(aUUID), "Entity doesn't exist in scene!");
		return Entity{ myIDToEntityMap.at(aUUID), this };
	}

	Entity Scene::TryGetEntityWithUUID(UUID aUUID)
	{
		if (auto it = myIDToEntityMap.find(aUUID); it != myIDToEntityMap.end())
		{
			return Entity{ it->second, this };
		}

		return Entity{};
	}

	void Scene::ParentEntity(Entity aEntity, Entity aParent)
	{
		if (aParent.IsDescendantOf(aEntity))
		{
			UnparentEntity(aParent);

			Entity newParent = aEntity.GetParent();
			if (newParent)
			{
				UnparentEntity(aEntity);
				ParentEntity(aParent, newParent);
			}
		}
		else
		{
			Entity previousParent = aEntity.GetParent();
			if (previousParent)
			{
				UnparentEntity(aEntity);
			}
		}

		aEntity.GetOrAddComponent<ParentComponent>().Parent = aParent;
		aParent.GetOrAddComponent<ChildrenComponent>().Children.emplace_back(aEntity);
	}

	void Scene::UnparentEntity(Entity aEntity)
	{
		Entity parent = aEntity.GetParent();
		if (!parent) return;

		auto& children = parent.GetComponent<ChildrenComponent>().Children;
		children.erase(std::remove(children.begin(), children.end(), aEntity), children.end());

		aEntity.RemoveComponent<ParentComponent>();
	}

	Entity Scene::Instantiate(std::shared_ptr<Assets::ModelAsset> aModel)
	{
		return InstantiateChild(aModel, {});
	}

	Entity Scene::InstantiateChild(std::shared_ptr<Assets::ModelAsset> aModel, Entity aParent)
	{
		const auto& modelData = aModel->GetData();

		if (!modelData.IsValid()) return {};

		std::vector<Entity> entityMap(modelData.Hierarchy.size());

		for (size_t i = 0; i < modelData.Hierarchy.size(); ++i)
		{
			const auto& node = modelData.Hierarchy[i];

			if (!node.IsRoot() && node.MeshIndex == UINT32_MAX) continue;

			Entity entity;

			if (!node.IsRoot())
			{
				Entity parent = entityMap[node.Parent];
				entity = CreateChildEntity(parent, node.Name);
			}
			else
			{
				entity = (bool)aParent ? CreateChildEntity(aParent, node.Name) : CreateEntity(node.Name);
			}

			entity.GetComponent<TransformComponent>().LocalTransform = CU::Transform(node.LocalTransform);

			if (node.MeshIndex != UINT32_MAX)
			{
				entity.AddComponent<MeshRendererComponent>(modelData.MeshAssets[node.MeshIndex]);
			}

			entityMap[i] = entity;
		}

		return entityMap[0];
	}

	void Scene::PrintHierarchy()
	{
		auto view = myRegistry.view<IDComponent>();
		for (auto id : view)
		{
			Entity entity{ id, this };
			if (entity.HasComponent<ParentComponent>()) continue;
			PrintHierarchyRecursive(entity);
		}
	}

	void Scene::PrintHierarchyRecursive(Entity aEntity, uint32_t aDepth)
	{
		std::string indent;
		for (size_t i = 0; i < aDepth; i++)
		{
			indent.append("  ");
		}
		LOG_DEBUG("{}{}", indent, aEntity.GetName());

		if (!aEntity.HasComponent<ChildrenComponent>()) return;

		auto& childrenComponent = aEntity.GetComponent<ChildrenComponent>();
		for (auto child : childrenComponent.Children)
		{
			PrintHierarchyRecursive(Entity{ child, this }, aDepth + 1);
		}
	}

	CU::Matrix4x4f Scene::GetWorldSpaceTransformMatrix(Entity aEntity)
	{
		CU::Matrix4x4f transform = CU::Matrix4x4f::Identity;

		Entity parent = aEntity.GetParent();
		if (parent)
		{
			transform = GetWorldSpaceTransformMatrix(parent);
		}
		return aEntity.GetComponent<TransformComponent>().LocalTransform.GetMatrix() * transform;
	}
}
