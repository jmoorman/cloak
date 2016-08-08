// Cloak.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "GraphicsContext.h"
#include "Mesh.h"

static Mesh *g_pyramidMesh = nullptr;

void initScene()
{
	g_pyramidMesh = new Mesh();

	bool success = g_pyramidMesh->loadFromObj("../data/meshes/pyramid.obj");

}

int main()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	SDL_Window *window = SDL_CreateWindow("MyWindow", 800, 200, 640, 480, SDL_WINDOW_SHOWN);
	assert(window != nullptr);

	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (SDL_GetWindowWMInfo(window, &info)) {
		const char *subsystem = "an unknown system!";
		switch (info.subsystem) {
			case SDL_SYSWM_WINDOWS: subsystem = "Microsoft Windows(TM)"; break;
			case SDL_SYSWM_X11:		subsystem = "X Window System"; break;
			default: break;
		}

		printf("This program is running SDL version %d.%d.%d on %s\n\n",
			(int)info.version.major,
			(int)info.version.minor,
			(int)info.version.patch,
			subsystem);
	}
	else
	{
		printf("Couldn't get window information: %s\n", SDL_GetError());
	}

	GraphicsContext graphicsContext;
	graphicsContext.init(GetModuleHandle(NULL), info.info.win.window);

	initScene();

	U32 lastFrameTime = SDL_GetTicks();
	bool done = false;
	while (!done) {
		U32 currentFrameTime = SDL_GetTicks();
		U32 elapsedMillis = currentFrameTime - lastFrameTime;

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT) {
				done = true;
			}
			//std::cout << "FPS: " << (1000.f / elapsedMillis) << "(" << elapsedMillis << "ms)" << std::endl;
		}		

		graphicsContext.updateUniformBuffer(elapsedMillis);
		graphicsContext.drawFrame();

		lastFrameTime = currentFrameTime;
		Sleep(1); //remove this once we actually have some frame time
	}
	graphicsContext.destroy();
	SDL_DestroyWindow(window);

    return 0;
}

