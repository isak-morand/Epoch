#pragma once
#include <vector>
#include <memory>
#include "EpochCore/Assert.h"
#include "Layer.h"

namespace Epoch
{
	class LayerStack
	{
	public:
		LayerStack() = default;
		~LayerStack() = default;

		UUID PushLayer(std::unique_ptr<Layer> aLayer);
		UUID PushOverlay(std::unique_ptr<Layer> aOverlay);

		void PopLayer(UUID aID);
		void PopOverlay(UUID aID);

		Layer* operator[](size_t index)
		{
			EPOCH_ASSERT(index < myLayers.size(), "Out of scope!");
			return myLayers[index].get();
		}

		const Layer* operator[](size_t index) const
		{
			EPOCH_ASSERT(index < myLayers.size(), "Out of scope!");
			return myLayers[index].get();
		}

		size_t Size() const { return myLayers.size(); }

		auto begin() { return myLayers.begin(); }
		auto end() { return myLayers.end(); }

		auto rbegin() { return myLayers.rbegin(); }
		auto rend() { return myLayers.rend(); }

	private:
		std::vector<std::unique_ptr<Layer>> myLayers;
		unsigned int myLayerInsertIndex = 0;
	};
}
