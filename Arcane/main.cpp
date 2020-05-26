#include "arcpch.h"
#include "Core/Application.h"
#include "Core/Core.h"
#include "Core/Logger.h"
#include "Layers/ImGuiLayer.h"

int main(int argc, char **argv)
{
	// Pre-Engine Initialization
	Arcane::Logger::GetInstance();
	ARC_LOG_INFO("Initialized Logger");

	Arcane::Application::GetInstance().PushOverlay(new Arcane::ImGuiLayer());
	Arcane::Application::GetInstance().Run();

	return EXIT_SUCCESS;
}
