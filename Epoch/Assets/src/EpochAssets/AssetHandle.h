#pragma once
#include <EpochCore/UUID.h>

namespace Epoch
{
	using AssetHandle = UUID;

	template<typename T>
	class TypedAssetHandle
	{
	public:
		TypedAssetHandle() = default;
		explicit TypedAssetHandle(AssetHandle aId) : myID(aId) {}

		AssetHandle Get() const { return myID; }

		bool IsValid() const { return myID != 0; }

		bool operator==(const TypedAssetHandle<T>& other) const
		{
			return myID == other.myID;
		}

		operator AssetHandle() const { return myID; }
		
	private:
		AssetHandle myID;
	};
}
