#pragma once
#include <string>
#include "EpochCore/Events/Event.h"
#include "EpochCore/UUID.h"

namespace Epoch
{
	class Layer
	{
	public:
		Layer(const std::string& aName = "Layer") : myDebugName(aName), myID(UUID()) {}
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnRenderImGui() {}
		virtual void OnEvent(Event& aEvent) {}

		UUID GetID() const { return myID; }
		const std::string& GetName() const { return myDebugName; }

	private:
		const UUID myID;
		const std::string myDebugName;
	};
}
