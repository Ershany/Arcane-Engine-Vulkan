#include "arcpch.h"
#include "Core/Application.h"
#include "Core/Core.h"

namespace Arcane
{
	Arcane::Application* CreateApplication();
}

int main(int argc, char **argv)
{
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
