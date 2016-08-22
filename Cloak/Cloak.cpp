// Cloak.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "AnimatedMesh.h"
#include "Camera.h"
#include "GraphicsContext.h"
#include "Mesh.h"

static Mesh *g_pyramidMesh = nullptr;
#define BOB_ROWS 10
#define BOB_COLS 10
#define BOB_COUNT (BOB_ROWS * BOB_COLS)
static AnimatedMesh *g_bobLampArray[BOB_COUNT];

void initScene(GraphicsContext *graphicsContext)
{
	g_pyramidMesh = new Mesh();

	bool success = g_pyramidMesh->loadFromObj("../data/meshes/pyramid.obj");

	int count = 0;
	for (int i = 0; i < BOB_COLS; i++)
	{
		for (int j = 0; j < BOB_ROWS; j++)
		{
			AnimatedMesh *bob = new AnimatedMesh();
			bob->loadModel("../data/models/boblamp.md5mesh");
			Animation *animation = new Animation();
			animation->loadAnimation("../data/animations/boblamp.md5anim");
			bob->setAnimation(animation);
			bob->setPosition(glm::vec3(i * 4, 0.f, j * 4));
			bob->rotateBy(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
			bob->setScale(glm::vec3(0.1f, 0.1f, 0.1f));
			bob->update(rand() % 5000);
			graphicsContext->createCommandBuffer(bob);
			g_bobLampArray[count++] = bob;
		}
	}
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
	camera.setPosition(glm::vec3(BOB_COLS * 2.5, 15.f, BOB_ROWS * 5.f));

	GraphicsContext graphicsContext;
	graphicsContext.init(GetModuleHandle(NULL), info.info.win.window);

	initScene(&graphicsContext);

	SceneConstantBuffer perFrameCB = {};
	perFrameCB.viewMatrix = camera.getViewMatrix();
	perFrameCB.projectionMatrix = camera.getProjectionMatrix();
	perFrameCB.lightDirection = glm::normalize(glm::vec4(0.f, -0.5, 0.5, 0.f));
	perFrameCB.lightColor = glm::vec4(1.f, 1.f, 1.f, 0.25f);

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
			std::cout << "FPS: " << (1000.f / elapsedMillis) << "(" << elapsedMillis << "ms)" << std::endl;
		}

		camera.moveBy(glm::vec3(elapsedMillis / 100.f, 0.f, 0.f));
		camera.lookAt(glm::vec3(7.5f, 0.f, 5.f));
		perFrameCB.viewMatrix = camera.getViewMatrix();

		graphicsContext.updateSceneConstantBuffer(perFrameCB);

		for (int i = 0; i < BOB_COUNT; i++)
		{
			AnimatedMesh *bob = g_bobLampArray[i];
			bob->update(elapsedMillis);
			graphicsContext.updateConstantBuffer((void *)&bob->buildModelMatrix(), sizeof(ObjectConstantBuffer), bob->mObjectConstantBuffer);
			graphicsContext.updateConstantBuffer(bob->getBoneMatrices().data(), bob->getBoneMatrices().size() * sizeof(glm::mat4), bob->mAnimationConstantBuffer);
		}

		graphicsContext.drawFrame();

		lastFrameTime = currentFrameTime;
		Sleep(1); //remove this once we actually have some frame time
	}
	graphicsContext.destroy();
	SDL_DestroyWindow(window);

    return 0;
}

