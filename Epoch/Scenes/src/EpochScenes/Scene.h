#pragma once
#include "Entity.h"

namespace Epoch::Assets
{
	class ModelAsset;
}

namespace Epoch::Scenes
{
	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(std::string_view aName = "New Entity");
		Entity CreateChildEntity(Entity aParent, std::string_view aName = "New Entity");

		Entity GetEntityWithUUID(UUID aUUID);
		Entity TryGetEntityWithUUID(UUID aUUID);

		void ParentEntity(Entity aEntity, Entity aParent);
		void UnparentEntity(Entity aEntity);

		Entity Instantiate(std::shared_ptr<Assets::ModelAsset> aModel);
		Entity InstantiateChild(std::shared_ptr<Assets::ModelAsset> aModel, Entity aParent);

		template<typename... ComponentTypse>
		auto GetAllEntitiesWith() { return myRegistry.view<ComponentTypse...>(); }

		//TEMP
		void PrintHierarchy();
		void PrintHierarchyRecursive(Entity aEntity, uint32_t aDepth = 0);

	private:


	private:
		entt::registry myRegistry;

		std::unordered_map<UUID, entt::entity> myIDToEntityMap;
		std::unordered_map<entt::entity, UUID> myEntityToIDMap;


		friend class Entity;
	};
}

#include "EntityTemplates.h"
