#pragma once
#include <EpochCore/Assert.h>
#include "Entity.h"

namespace Epoch::Scenes
{
	template<typename T, typename... Args>
	T& Entity::AddComponent(Args&&... args)
	{
		EPOCH_ASSERT(!HasComponent<T>(), "Entity already has component!");
		T& component = myScene->myRegistry.emplace<T>(myEntityHandle, std::forward<Args>(args)...);
		return component;
	}

	template<typename T>
	void Entity::RemoveComponent()
	{
		EPOCH_ASSERT(HasComponent<T>(), "Entity does not have component!");
		myScene->myRegistry.remove<T>(myEntityHandle);
	}

	template<typename T>
	bool Entity::HasComponent() const
	{
		return myScene->myRegistry.all_of<T>(myEntityHandle);
	}

	template<typename...T>
	bool Entity::HasAny() const
	{
		return myScene->myRegistry.any_of<T...>(myEntityHandle);
	}

	template<typename T>
	T& Entity::GetComponent()
	{
		EPOCH_ASSERT(HasComponent<T>(), "Entity does not have component!");
		return myScene->myRegistry.get<T>(myEntityHandle);
	}

	template<typename T>
	const T& Entity::GetComponent() const
	{
		EPOCH_ASSERT(HasComponent<T>(), "Entity does not have component!");
		return myScene->myRegistry.get<T>(myEntityHandle);
	}

	template<typename T>
	T& Entity::GetOrAddComponent()
	{
		if (!HasComponent<T>())
		{
			return AddComponent<T>();
		}

		return GetComponent<T>();
	}
}
