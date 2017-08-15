#include <string>
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include "res_path.h"
#include "cleanup.h"


const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

/*
* Log an SDL error with some error message to the output stream of our choice
* @param os The output stream to write the message too
* @param msg The error message to write, format will be msg error: SDL_GetError()
*/
void logSDLError(std::ostream &os, const std::string &msg) {
	os << msg << " error: " << SDL_GetError() << std::endl;
}
/*
* Loads an image into a texture on the rendering device
* @param file The image file to load
* @param ren The renderer to load the texture onto
* @return the loaded texture, or nullptr if something went wrong.
*/
SDL_Texture* loadTexture(const std::string &file, SDL_Renderer *ren) {
	SDL_Texture *texture = IMG_LoadTexture(ren, file.c_str());
	if (texture == nullptr) {
		logSDLError(std::cout, "LoadTexture");
	}
	return texture;
}
/*
* Draw an SDL_Texture to an SDL_Renderer at some destination rect
* taking a clip of the texture if desired
* @param tex The source texture we want to draw
* @param rend The renderer we want to draw too
* @param dst The destination rectangle to render the texture too
* @param clip The sub-section of the texture to draw (clipping rect)
*		default of nullptr draws the entire texture
*/
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, SDL_Rect dst, SDL_Rect *clip = nullptr) {
	SDL_RenderCopy(ren, tex, clip, &dst);
}
/*
* Draw an SDL_Texture to an SDL_Renderer at position x, y, preserving
* the texture's width and height and taking a clip of the texture if desired
* If a clip is passed, the clip's width and height will be used instead of the texture's
* @param tex The source texture we want to draw
* @param rend The renderer we want to draw too
* @param x The x coordinate to draw too
* @param y The y coordinate to draw too
* @param clip The sub-section of the texture to draw (clipping rect)
*		default of nullptr draws the entire texture
*/
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
	//Start up SDL and make sure it went ok
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		logSDLError(std::cout, "SDL_Init");
		return 1;
	}

	//Setup our window and renderer
	SDL_Window *window = SDL_CreateWindow("Lesson 5", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
	SDL_Texture *image = loadTexture("C:/Users/juanp/Source/Repos/sdl2test/sdl2test/image.png", renderer);
	if (image == nullptr) {
		cleanup(image, renderer, window);
		IMG_Quit();
		SDL_Quit();
		return 1;
	}

	//iW and iH are the clip width and height
	//We'll be drawing only clips so get a center position for the w/h of a clip
	int iW = 100, iH = 100;
	float x = 0;
	float y = SCREEN_HEIGHT - iH;
	float Velx = 0;
	float Vely = 0;
	float g = -0.75;
	int jumpnumber = 0;
	bool Leftpress = 0;
	bool Rightpress = 0;
	bool Movingright = 0;
	bool Movingleft = 0;

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
					break;
				case SDLK_RIGHT:
					Rightpress = 1;
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
					jumpnumber = jumpnumber+1;
					break;
				case SDLK_LEFT:
					Leftpress = 0;
					break;
				case SDLK_RIGHT:
					Rightpress = 0;
					break;
				default:
					break;
				}
			}
		}

		//Movement logic goes here
		if (x <= 0) { Leftpress = 0; }
		if (x >= 1280 - iW) { Rightpress = 0; }
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

		if (y >= 720 - iH) { g = 0; } //Otherwise the player will fall off the screen in the down (y +) direction
		else { g = -0.75; } //TODO: make this modular and changable
		Vely = Vely - g;
		x = x + Velx;
		y = y + Vely;
		if (y >= 720 - iH) { y = 720 - iH; }

		//Rendering
		SDL_RenderClear(renderer);
		//Draw the image
		renderTexture(image, renderer, x, y, &clips[useClip]);
		//Update the screen
		SDL_RenderPresent(renderer);

	}
	//Clean up
	cleanup(image, renderer, window);
	IMG_Quit();
	SDL_Quit();

	return 0;
}