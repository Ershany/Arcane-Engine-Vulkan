#pragma once

// TODO: This should be moved to renderer class, for now this will do
enum class SwapchainPresentMode
{
	DOUBLE_BUFFER,
	TRIPLE_BUFFER,
};
const SwapchainPresentMode g_SwapchainPresentMode = SwapchainPresentMode::TRIPLE_BUFFER;
