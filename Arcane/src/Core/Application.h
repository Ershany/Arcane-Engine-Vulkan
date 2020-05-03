#pragma once

#include "Core/Core.h"
#include "Defs.h"
#include "Events/Event.h"
#include "Core/Window.h"
#include "Renderer/VulkanAPI.h"

namespace Arcane
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	private:
		void Loop();
		void Cleanup();
	private:
		Window m_Window;

		VulkanAPI m_Vulkan;
	};
}
