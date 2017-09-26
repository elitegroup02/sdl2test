#include <string>
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
#include "res_path.h"
#include "cleanup.h"

using namespace std;

void logSDLError(std::ostream &os, const std::string &msg) {
	os << msg << " error: " << SDL_GetError() << std::endl;
}

SDL_Texture* loadTexture(const string &file, SDL_Renderer *ren) {
	SDL_Texture *texture = IMG_LoadTexture(ren, file.c_str());
	if (texture == nullptr) {
		logSDLError(cout, "LoadTexture");
	}
	return texture;
}

void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, SDL_Rect dst, SDL_Rect *clip = nullptr) {
	SDL_RenderCopy(ren, tex, clip, &dst);
}

void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, SDL_Rect *clip = nullptr) {
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	if (clip != nullptr) {
		dst.w = clip->w;
		dst.h = clip->h;
	}
	else {
		SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
	}
	renderTexture(tex, ren, dst, clip);
}

int main(int, char**) {

	int S_W;
	int S_H;

	cout << "Input the screen width: ";
	cin >> S_W;
	cout << "\nInput screen height: ";
	cin >> S_H;
	cout << "\n";

	//Start up SDL and make sure it went ok
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		logSDLError(std::cout, "SDL_Init");
		return 1;
	}

	//Setup our window and renderer
	SDL_Window *window = SDL_CreateWindow("Game", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, S_W, S_H, SDL_WINDOW_SHOWN);
	if (window == nullptr) {
		logSDLError(std::cout, "CreateWindow");
		SDL_Quit();
		return 1;
	}
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr) {
		logSDLError(std::cout, "CreateRenderer");
		cleanup(window);
		SDL_Quit();
		return 1;
	}
	SDL_Texture *player = loadTexture("C:/Users/juanp/Source/Repos/sdl2test/sdl2test/image.png", renderer);
	SDL_Texture *Sun = loadTexture("C:/Users/juanp/Source/Repos/sdl2test/sdl2test/Sun.png", renderer);
	SDL_Texture *Clouds = loadTexture("C:/Users/juanp/Source/Repos/sdl2test/sdl2test/Clouds.png", renderer);
	SDL_Texture *Base = loadTexture("C:/Users/juanp/Source/Repos/sdl2test/sdl2test/Base.png", renderer);
	SDL_Texture *Mountains = loadTexture("C:/Users/juanp/Source/Repos/sdl2test/sdl2test/Mountains.png", renderer);
	SDL_Texture *Grass = loadTexture("C:/Users/juanp/Source/Repos/sdl2test/sdl2test/Grass.png", renderer);
	if (player == nullptr || Sun == nullptr || Clouds == nullptr || Base == nullptr || Mountains == nullptr || Grass == nullptr) {
		cleanup(player, Sun, Clouds, Base, Mountains, Grass, renderer, window);
		IMG_Quit();
		SDL_Quit();
		return 1;
	}

	//iW and iH are the clip width and height
	//We'll be drawing only clips so get a center position for the w/h of a clip
	int iW = 100, iH = 100;
	float xPlayer = 0; //The initial x position
	float yPlayer = S_H - iH; //The initial height is the floor
	float Velx = 0;
	float Vely = 0;
	float g = -0.80;
	int jumpnumber = 0;
	int Cloudsloop = 0;
	int Mountainsloop = 0;
	int Grassloop = 0;
	int Sunloop = 0;
	bool Leftpress = 0;
	bool Rightpress = 0;
	bool Rightpressaux = 0;
	bool Leftpressaux = 0;
	bool Movingright = 0;
	bool Movingleft = 0;
	float xBase = 0;
	float yBase = 0;
	float xMountains = 0;
	float yMountains = 0;
	float xClouds = 0;
	float yClouds = 0;
	float xSun = 0;
	float ySun = 0;
	float xGrass = 0;
	float yGrass = 0;

	//Setup the clips for our image
	SDL_Rect clips[4];
	//Since our clips our uniform in size we can generate a list of their
	//positions using some math (the specifics of this are covered in the lesson)
	for (int i = 0; i < 4; ++i) {
		clips[i].x = i / 2 * iW;
		clips[i].y = i % 2 * iH;
		clips[i].w = iW;
		clips[i].h = iH;
	}
	//Specify a default clip to start with
	int useClip = 0;

	SDL_Event e;
	bool quit = false;
	while (!quit) {
		//MAIN LOOP
		//Event Polling
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}
			//Use number input to select which clip should be drawn
			if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
				case SDLK_SPACE:
					Vely = -20;
					break;
				case SDLK_LEFT:
					Leftpress = 1;
					Leftpressaux = 1;
					break;
				case SDLK_RIGHT:
					Rightpress = 1;
					Rightpressaux = 1;
					break;
				case SDLK_1:
					useClip = 0;
					break;
				case SDLK_2:
					useClip = 1;
					break;
				case SDLK_3:
					useClip = 2;
					break;
				case SDLK_4:
					useClip = 3;
					break;
				case SDLK_ESCAPE:
					quit = true;
					break;
				default:
					break;
				}
			}
			if (e.type == SDL_KEYUP) {
				switch (e.key.keysym.sym) {
				case SDLK_SPACE:
					jumpnumber = jumpnumber + 1;
					break;
				case SDLK_LEFT:
					Leftpress = 0;
					Leftpressaux = 0;
					break;
				case SDLK_RIGHT:
					Rightpress = 0;
					Leftpressaux = 0;
					break;
				default:
					break;
				}
			}
		}

		//Movement logic goes here
		if (xPlayer <= 0) { Leftpress = 0; } //This means only the opposite button to the edge of the screen being touched can be pressed
		if (xPlayer >= (1280 - iW) / 3.5) { Rightpress = 0; } //1280 -250 pixels that make a buffer for the rest of the level
		if (Rightpress == 1 && Leftpress == 1) //If both are pressed at the same time then stop
		{
			Velx = 0;
		}
		else //Else, only one of them is pressed. Pick the pressed side and give it a constant speed, also flag it as moving
		{
			if (Rightpress == 1) { Velx = 3.5; Movingright = 1; }
			if (Leftpress == 1) { Velx = -3.5; Movingleft = 1; }
		}
		if (Movingleft == 1 && Leftpress == 0) { Velx = 0; Movingleft = 0; } //If either is moving and the button is released, reset the speed and flag
		if (Movingright == 1 && Rightpress == 0) { Velx = 0; Movingright = 0; }

		if (yPlayer >= 720 - iH) { g = 0; } //Otherwise the player will fall off the screen in the down (y +) direction
		else { g = -0.75; } //TODO: make this modular and changeable
		Vely = Vely - g;
		xPlayer = xPlayer + Velx;
		yPlayer = yPlayer + Vely;
		if (yPlayer >= 720 - iH) { yPlayer = 720 - iH; }

		if (xPlayer >= (1280 - iW) / 3.5 && Rightpressaux == true)
		{
			xMountains = xMountains - (3.5) / 1.5;
			xGrass = xGrass - 3.5;
			xClouds = xClouds - (3.5) / 3;
			xSun = xSun - (3.5) / 12;

		}

		if (xPlayer <= 0 && Leftpressaux == true)
		{
			xMountains = xMountains + (3.5) / 1.5;
			xGrass = xGrass + 3.5;
			xClouds = xClouds + (3.5) / 3;
			xSun = xSun - (3.5) / 12;
		}

		if (xSun <= -S_W) { xSun = 0; }
		if (xGrass <= -S_W) { xGrass = 0; }
		if (xMountains <= -S_W) { xMountains = 0; }
		if (xClouds <= -S_W) { xClouds = 0; }

		//Rendering
		SDL_RenderClear(renderer);
		//Draw the image
		renderTexture(Base, renderer, xBase, yBase);
		renderTexture(Sun, renderer, xSun, ySun);
		renderTexture(Clouds, renderer, xClouds, yClouds);
		renderTexture(Mountains, renderer, xMountains, yMountains);
		renderTexture(Grass, renderer, xGrass, yGrass);
		renderTexture(player, renderer, xPlayer, yPlayer, &clips[useClip]);
		//Update the screen
		SDL_RenderPresent(renderer);

	}
	//Clean up
	cleanup(player, Sun, Clouds, Base, Mountains, Grass, renderer, window);
	IMG_Quit();
	SDL_Quit();

	return 0;
}