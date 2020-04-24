#pragma once

#include "arcpch.h"


#ifdef ARC_PLATFORM_WINDOWS

extern Arcane::Application* Arcane::CreateApplication();

int main(int argc, char **argv)
{
	auto application = Arcane::CreateApplication();
	try
	{
		application->Run();
	}
	catch (const std::exception & e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

#endif
