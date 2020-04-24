#include <Arcane.h>

class Game : public Arcane::Application
{
public:
	Game() {}
	~Game() {}
};

Arcane::Application* Arcane::CreateApplication()
{
	return new Game();
}
