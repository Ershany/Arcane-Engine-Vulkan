#pragma once

#ifdef ARC_PLATFORM_WINDOWS
#else
	#error Arcane only supports Windows
#endif

#ifdef ARC_DEBUG
#define ARC_ASSERT(x, ...) { if(!(x)) { ARC_LOG_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
#define ARC_ASSERT(x, ...) { if(!(x)) { ARC_LOG_ERROR("Assertion Failed: {0}", __VA_ARGS__); } }
#endif
