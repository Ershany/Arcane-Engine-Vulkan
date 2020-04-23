#include "arcpch.h"
#include "FileUtils.h"

std::string FileUtils::ReadFile(const std::string &filepath)
{
	std::ifstream ifs(filepath, std::ios::in, std::ios::binary);

	std::string result;

	if (ifs)
	{
		result = std::string((std::istreambuf_iterator<char>(ifs)),
			(std::istreambuf_iterator<char>()));
		ifs.close();
	}
	else
	{
		throw std::runtime_error("Could not read file path");
	}

	return result;
}
