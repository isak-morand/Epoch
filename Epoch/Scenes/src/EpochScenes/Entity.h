#pragma once
#include <entt/entt.hpp>
#include "Components.h"

namespace Epoch::Scenes
{
	class Scene;

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity aHandle, Scene* aScene) : myEntityHandle(aHandle), myScene(aScene) {}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args);

		template<typename T>
		void RemoveComponent();

		template<typename T>
		bool HasComponent() const;

		template<typename...T>
		bool HasAny() const;

		template<typename T>
		T& GetComponent();

		template<typename T>
		const T& GetComponent() const;

		template<typename T>
		T& GetOrAddComponent();

		UUID GetUUID() { return GetComponent<IDComponent>().ID; }
		const std::string& GetName() { return GetComponent<NameComponent>().Name; }

		bool HasParent() const { return HasComponent<ParentComponent>(); }
		Entity GetParent() const;
		void SetParent(Entity aParent);

		void RemoveChild(Entity aChild);

		bool IsAncestorOf(Entity aEntity) const;
		bool IsDescendantOf(Entity aEntity) const { return aEntity.IsAncestorOf(*this); }

		operator bool() const { return myEntityHandle != entt::null; }
		operator entt::entity() const { return myEntityHandle; }
		operator uint32_t() const { return (uint32_t)myEntityHandle; }

		bool operator==(const Entity& aOther) const { return myEntityHandle == aOther.myEntityHandle && myScene == aOther.myScene; }
		bool operator!=(const Entity& aOther) const { return !(*this == aOther); }

	private:
		entt::entity myEntityHandle{ entt::null };
		Scene* myScene = nullptr;

		friend class Scene;
	};
}
