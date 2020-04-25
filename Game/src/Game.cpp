#include "Arcane.h"

class Game : public Arcane::Application
{
public:
	Game() {}
	~Game() {}
};

Arcane::Application* Arcane::CreateApplication()
{
	std::cout << "in GAME" << std::endl;
	return new Game();
}
