#include "arcpch.h"
#include "Core/Application.h"
#include "Core/Core.h"
#include "Core/Logger.h"

int main(int argc, char **argv)
{
	// Pre-Engine Initialization
	Arcane::Logger::GetInstance();
	ARC_LOG_INFO("Initialized Logger");

	Arcane::Application application;
	application.Run();

	return EXIT_SUCCESS;
}
