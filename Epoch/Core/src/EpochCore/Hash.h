#pragma once
#include <string>

namespace Epoch::Hash
{
	static constexpr uint64_t GenerateFNVHash(std::string_view aStr)
	{
		constexpr uint64_t FNV_offset_basis = 14695981039346656037ULL;
		constexpr uint64_t FNV_prime = 1099511628211ULL;

		uint64_t hash = FNV_offset_basis;
		for (char c : aStr)
		{
			hash ^= static_cast<uint64_t>(c);
			hash *= FNV_prime;
		}
		return hash;
	}
}
