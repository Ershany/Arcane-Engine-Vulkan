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
