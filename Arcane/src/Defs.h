#pragma once

#if defined(_MSC_VER)
	#define DISABLE_DLL_WARNING __pragma(warning(disable:4251))
#elif defined(__GNUC__) || defined(__clang__)
	#define DISABLE_DLL_WARNING
#else
	#define DISABLE_DLL_WARNING
#endif
DISABLE_DLL_WARNING

// TODO: This should be moved to renderer class, for now this will do
enum class SwapchainPresentMode
{
	VSYNC_OFF,
	VSYNC_DOUBLE_BUFFER,
	VSYNC_TRIPLE_BUFFER,
};
const SwapchainPresentMode g_SwapchainPresentMode = SwapchainPresentMode::VSYNC_TRIPLE_BUFFER;
