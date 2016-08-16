// Cloak.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "AnimatedMesh.h"
#include "Camera.h"
#include "GraphicsContext.h"
#include "Mesh.h"

static Mesh *g_pyramidMesh = nullptr;
static AnimatedMesh *g_bobLamp = nullptr;
static AnimatedMesh *g_bobLamp2 = nullptr;

void initScene(GraphicsContext *graphicsContext)
{
	g_pyramidMesh = new Mesh();

	bool success = g_pyramidMesh->loadFromObj("../data/meshes/pyramid.obj");

	g_bobLamp = new AnimatedMesh();
	success = g_bobLamp->loadModel("../data/models/boblamp.md5mesh");
	Animation *animation = new Animation();
	animation->loadAnimation("../data/animations/boblamp.md5anim");
	g_bobLamp->setAnimation(animation);

	graphicsContext->createCommandBuffer(g_bobLamp);

	g_bobLamp2 = new AnimatedMesh();
	success = g_bobLamp2->loadModel("../data/models/boblamp.md5mesh");
	Animation *animation2 = new Animation();
	animation2->loadAnimation("../data/animations/boblamp.md5anim");
	g_bobLamp->setAnimation(animation2);
	
	//graphicsContext->createCommandBuffer(g_bobLamp2);
}

int main()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	const int width = 640;
	const int height = 480;
	SDL_Window *window = SDL_CreateWindow("MyWindow", 800, 200, width, height, SDL_WINDOW_SHOWN);
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

	Camera camera;
	camera.setPerspective(45.f, width / (float)height, 0.1f, 1000.f);
	camera.setPosition(glm::vec3(0.f, 0.f, -150.f));
	camera.setDirection(glm::vec3(0.f, 0.f, 1.f));

	GraphicsContext graphicsContext;
	graphicsContext.init(GetModuleHandle(NULL), info.info.win.window);

	initScene(&graphicsContext);
	g_bobLamp->setPosition(glm::vec3(0.f, 0.f, -60.f));
	g_bobLamp->rotate(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));

	PerFrameConstantBuffer perFrameCB = {};
	perFrameCB.modelMatrix = g_bobLamp->getModelMatrix();
	perFrameCB.viewMatrix = camera.getViewMatrix();
	perFrameCB.projectionMatrix = camera.getProjectionMatrix();

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

		camera.move(glm::vec3(elapsedMillis / 10.f,	0.f, 0.f));
		camera.lookAt(glm::vec3(0.f, 30.f, 0.f));
		perFrameCB.viewMatrix = camera.getViewMatrix();

		graphicsContext.updatePerFrameConstantBuffer(perFrameCB);
		g_bobLamp->update(elapsedMillis);
		graphicsContext.updateConstantBuffer(g_bobLamp->getBoneMatrices().data(), sizeof(AnimationConstantBuffer), g_bobLamp->mAnimationConstantBuffer);
		graphicsContext.drawFrame();

		lastFrameTime = currentFrameTime;
		Sleep(1); //remove this once we actually have some frame time
	}
	graphicsContext.destroy();
	SDL_DestroyWindow(window);

    return 0;
}

