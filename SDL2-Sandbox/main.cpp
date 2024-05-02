#include "Engine.h"
#include "SimpleSock/socklib.h"

int main(int argc, char* argv[])
{
	SockLibInit();
	Funny::Engine* engine = nullptr;
	engine = Funny::Engine::createInstance();

	if (argc > 1)
	{
		engine->init("Funny", 0, 0, true);
	}

	else
	{
		engine->init("Funny", 640, 480, false);
		//engine->init("Funny", 0, 0, true);
	}

	while (engine->gameLoop())
	{

	}

	engine->close();
	delete(engine);
	SockLibShutdown();
	return 0;
}