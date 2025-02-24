#pragma once
#include <Epoch/Editor/EditorPanel.h>

namespace Epoch
{
	class ScriptEngineDebugPanel : public EditorPanel
	{
	public:
		ScriptEngineDebugPanel(const std::string& aName);
		~ScriptEngineDebugPanel() override = default;
		
		void OnImGuiRender(bool& aIsOpen) override;
		void OnProjectChanged(const std::shared_ptr<Project>& aProject) override;
	};
}
