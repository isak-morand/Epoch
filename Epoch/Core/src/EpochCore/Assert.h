#pragma once
#include "Log.h"

#define DEBUG_BREAK __debugbreak()

#ifdef _DEBUG
	#define EPOCH_ENABLE_ASSERTS
#endif

#ifdef EPOCH_ENABLE_ASSERTS
	#define ASSERT_MESSAGE_INTERNAL(...)  ::Epoch::Log::PrintAssertMessage("Assertion Failed" __VA_OPT__(,) __VA_ARGS__)
	#define EPOCH_ASSERT(condition, ...) { if(!(condition)) { ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); DEBUG_BREAK; } }
#else
	#define EPOCH_ASSERT(condition, ...)
#endif

#define ENSURE_MESSAGE_INTERNAL(...)  ::Epoch::Log::PrintAssertMessage("Ensure Failed" __VA_OPT__(,) __VA_ARGS__)
#define EPOCH_ENSURE(condition, ...) { if(!(condition)) { ENSURE_MESSAGE_INTERNAL(__VA_ARGS__); DEBUG_BREAK; } }
