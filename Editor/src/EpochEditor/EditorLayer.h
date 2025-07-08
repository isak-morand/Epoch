#pragma once
#include <memory>
#include <EpochEngine/AppLayer/Layer.h>

namespace Epoch::Scenes
{
	class Scene;
}

namespace Epoch::Editor
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer() : Layer("Editor Layer") {}
		~EditorLayer() override = default;

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
		void OnRenderImGui() override;
		void OnEvent(Event& aEvent) override;

	private:
		std::shared_ptr<Scenes::Scene> myScene;
	};
}
