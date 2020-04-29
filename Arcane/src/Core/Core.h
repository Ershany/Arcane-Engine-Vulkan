#pragma once

#ifdef ARC_PLATFORM_WINDOWS
	#ifdef ARC_BUILD_DLL
		#define ARCANE_API __declspec(dllexport)
	#else
		#define ARCANE_API __declspec(dllimport)
	#endif
#else
	#error Arcane only supports Windows
#endif

// TODO: Just log instead of not in debug
#ifdef ARC_DEBUG
#define ARC_ENGINE_ASSERT(x, ...) { if(!(x)) { ARC_ENGINE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#define ARC_GAME_ASSERT(x, ...) { if(!(x)) { ARC_GAME_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
#define ARC_ENGINE_ASSERT(x, ...)
#define ARC_GAME_ASSERT(x, ...)
#endif
