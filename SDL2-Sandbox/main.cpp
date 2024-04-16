#include "Engine.h"

int main(int argc, char* argv[])
{
	Funny::Engine* engine = nullptr;
	engine = Funny::Engine::createInstance();
	engine->init("Funny", 640, 480);

	while (engine->gameLoop())
	{

	}

	engine->close();
	delete(engine);
	return 0;
}