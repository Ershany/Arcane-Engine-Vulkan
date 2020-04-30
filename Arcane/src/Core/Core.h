#pragma once

// TODO: Just log instead of not in debug
#ifdef ARC_DEBUG
#define ARC_ENGINE_ASSERT(x, ...) { if(!(x)) { ARC_ENGINE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#define ARC_GAME_ASSERT(x, ...) { if(!(x)) { ARC_GAME_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
#define ARC_ENGINE_ASSERT(x, ...)
#define ARC_GAME_ASSERT(x, ...)
#endif
