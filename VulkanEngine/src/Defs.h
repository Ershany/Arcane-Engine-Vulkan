#pragma once

// TODO: This should be moved to renderer class, for now this will do
enum class SwapchainPresentMode
{
	VSYNC_OFF,
	VSYNC_DOUBLE_BUFFER,
	VSYNC_TRIPLE_BUFFER,
};
const SwapchainPresentMode g_SwapchainPresentMode = SwapchainPresentMode::VSYNC_TRIPLE_BUFFER;
