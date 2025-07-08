#include "epch.h"
#include "UUID.h"

namespace Epoch
{
	static std::random_device staticRandomDevice;
	static std::mt19937_64 staticEngine(staticRandomDevice());
	static std::uniform_int_distribution<uint64_t> staticUniformDistribution;

	UUID::UUID() : myUUID(staticUniformDistribution(staticEngine)) {}

	UUID::UUID(uint64_t aUID) : myUUID(aUID) {}
}
