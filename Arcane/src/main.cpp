#include "arcpch.h"

#include "Core/Application.h"

int main()
{
	Arcane::Application app;

	try
	{
		app.Run();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
