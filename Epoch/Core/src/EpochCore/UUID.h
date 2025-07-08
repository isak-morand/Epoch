#pragma once
#include <xhash>

namespace Epoch
{
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t aUID);
		UUID(const UUID&) = default;

		operator uint64_t() { return myUUID; }
		operator const uint64_t () const { return myUUID; }

	private:
		uint64_t myUUID;
	};
}

namespace std
{
	template <typename T> struct hash;

	template<>
	struct hash<Epoch::UUID>
	{
		size_t operator()(const Epoch::UUID& uuid) const
		{
			return hash<uint64_t>{}(uuid);
		}
	};
}
