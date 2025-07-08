#include "epch.h"
#include "LayerStack.h"

namespace Epoch
{
	UUID LayerStack::PushLayer(std::unique_ptr<Layer> aLayer)
	{
		UUID layerID = aLayer->GetID();
		aLayer->OnAttach();
		myLayers.emplace(myLayers.begin() + myLayerInsertIndex, std::move(aLayer));
		myLayerInsertIndex++;
		return layerID;
	}

	UUID LayerStack::PushOverlay(std::unique_ptr<Layer> aOverlay)
	{
		UUID layerID = aOverlay->GetID();
		aOverlay->OnAttach();
		myLayers.emplace_back(std::move(aOverlay));
		return layerID;
	}

	void LayerStack::PopLayer(UUID aID)
	{
		auto it = std::find_if(myLayers.begin(), myLayers.end(), [&](const std::unique_ptr<Layer>& ptr) { return ptr->GetID() == aID; });

		if (it != myLayers.end())
		{
			(*it)->OnDetach();
			myLayers.erase(it);
			myLayerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(UUID aID)
	{
		auto it = std::find_if(myLayers.begin() + myLayerInsertIndex, myLayers.end(), [&](auto& ptr) { return ptr->GetID() == aID; });

		if (it != myLayers.end())
		{
			(*it)->OnDetach();
			myLayers.erase(it);
		}
	}
}
