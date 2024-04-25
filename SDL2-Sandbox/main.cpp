#include "Engine.h"
#include "SimpleSock/socklib.h"

int main(int argc, char* argv[])
{
	SockLibInit();
	Funny::Engine* engine = nullptr;
	engine = Funny::Engine::createInstance();
	engine->init("Funny", 640, 480);

	while (engine->gameLoop())
	{

	}

	engine->close();
	delete(engine);
	SockLibShutdown();
	return 0;
}