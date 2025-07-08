#pragma once
#include "DeviceManager.h"
#include "SwapChain.h"

namespace Epoch::Rendering
{
	//TODO: REWORK
	struct RenderContext
	{
		DeviceManager* DeviceManager = nullptr;
		SwapChain* SwapChain = nullptr;

		static RenderContext& Get()
		{
			static RenderContext staticInstance;
			return staticInstance;
		}
	};
}
