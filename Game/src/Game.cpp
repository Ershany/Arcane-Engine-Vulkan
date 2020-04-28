#include <Arcane.h>

class Game : public Arcane::Application
{
public:
	Game() {}
	~Game() {}
};

Arcane::Application* Arcane::CreateApplication()
{
	ARC_GAME_FATAL("AHH");
	return new Game();
}
