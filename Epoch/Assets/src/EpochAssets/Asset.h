#pragma once
#include "AssetHandle.h"
#include "AssetTypes.h"

namespace Epoch::Assets
{
	enum class AssetState : uint8_t
	{
		CPUOnly,
		GPUReady,
		DirtyGPU
	};

	class Asset
	{
	public:
		Asset() = delete;
		explicit Asset(AssetHandle aHandle) : myHandle(aHandle) {}
		virtual ~Asset() = default;

		virtual AssetType GetAssetType() const = 0;

		AssetHandle GetHandle() const { return myHandle; }

		AssetState GetAssetState() const { return myState; }
		void SetAssetState(AssetState aState) { myState = aState; }

		bool operator==(const Asset& aOther) const { return myHandle == aOther.myHandle; }
		bool operator!=(const Asset& aOther) const { return myHandle != aOther.myHandle; }

	private:
		AssetHandle myHandle = 0;
		AssetState myState = AssetState::CPUOnly;
	};
}
