#include "Entity.h"
#include "Scene.h"

namespace Epoch::Scenes
{
	Entity Entity::GetParent() const
	{
		if (!HasParent()) return Entity{};
		return { GetComponent<ParentComponent>().Parent, myScene };
	}

	void Entity::SetParent(Entity aParent)
	{
		bool hasParent = HasParent();

		if (hasParent)
		{
			Entity currentParent = { GetComponent<ParentComponent>().Parent, myScene };
			if (currentParent == aParent)  return;

			currentParent.RemoveChild(*this);
		}

		if (!aParent)
		{
			if (hasParent) RemoveComponent<ParentComponent>();
			return;
		}

		if (!hasParent)
		{
			AddComponent<ParentComponent>(aParent);
		}
		else
		{
			GetComponent<ParentComponent>().Parent = aParent;
		}

		aParent.GetOrAddComponent<ChildrenComponent>().Children.emplace_back(myEntityHandle);
	}

	void Entity::RemoveChild(Entity aChild)
	{
		if (!HasComponent<ChildrenComponent>()) return;
		
		auto& children = GetComponent<ChildrenComponent>().Children;
		auto it = std::find(children.begin(), children.end(), aChild);
		if (it != children.end())
		{
			children.erase(it);
		}

		if (children.empty())
		{
			RemoveComponent<ChildrenComponent>();
		}
	}

	bool Entity::IsAncestorOf(Entity aEntity) const
	{
		if (!HasComponent<ChildrenComponent>())
		{
			return false;
		}

		const auto& children = GetComponent<ChildrenComponent>().Children;
		for (auto child : children)
		{
			if (child == aEntity)
			{
				return true;
			}

			if (Entity(child, myScene).IsAncestorOf(aEntity))
			{
				return true;
			}
		}

		return false;
	}
}
