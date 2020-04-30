#pragma once

#include "Window.h"
#include "Renderer/VulkanAPI.h"
#include "Core/Core.h"
#include "Defs.h"
#include "Events/Event.h"

namespace Arcane
{
	class Application
	{
	public:
		Application();
		virtual  ~Application();

		void Run();
	private:
		void Loop();
		void Cleanup();
	private:
		Window m_Window;

		VulkanAPI m_Vulkan;
	};
}
