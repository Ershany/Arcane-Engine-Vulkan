#include "arcpch.h"
#include "Core/Application.h"
#include "Core/Core.h"
#include "Core/Logger.h"

namespace Arcane
{
	Arcane::Application* CreateApplication();
}

int main(int argc, char **argv)
{
	// Pre-Engine Initialization
	Arcane::Logger::GetInstance();
	ARC_ENGINE_INFO("Initialized Logger");
	ARC_GAME_INFO("Initialized Logger");

	auto application = Arcane::CreateApplication();
	try
	{
		application->Run();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
