#include <iostream>
#include <string>
#include <SDL.h>
#include "res_path.h"
#include "cleanup.h"
#include <stdio.h>
#include <time.h>
using namespace std;

/*
* Lesson 2: Don't Put Everything in Main
*/
//Screen attributes
int SCREEN_WIDTH = 1280;
int SCREEN_HEIGHT = 720;

/*
* Log an SDL error with some error message to the output stream of our choice
* @param os The output stream to write the message too
* @param msg The error message to write, format will be msg error: SDL_GetError()
*/
void logSDLError( ostream &os, const  string &msg) {
	os << msg << " error: " << SDL_GetError() <<  endl;
}
/*
* Loads a BMP image into a texture on the rendering device
* @param file The BMP image file to load
* @param ren The renderer to load the texture onto
* @return the loaded texture, or nullptr if something went wrong.
*/
SDL_Texture* loadTexture(const  string &file, SDL_Renderer *ren) {
	SDL_Texture *texture = nullptr;
	//Load the image
	SDL_Surface *loadedImage = SDL_LoadBMP(file.c_str());
	//If the loading went ok, convert to texture and return the texture
	if (loadedImage != nullptr) {
		texture = SDL_CreateTextureFromSurface(ren, loadedImage);
		SDL_FreeSurface(loadedImage);
		//Make sure converting went ok too
		if (texture == nullptr) {
			logSDLError( cout, "CreateTextureFromSurface");
		}
	}
	else {
		logSDLError( cout, "LoadBMP");
	}
	return texture;
}
/*
* Draw an SDL_Texture to an SDL_Renderer at position x, y, preserving
* the texture's width and height
* @param tex The source texture we want to draw
* @param ren The renderer we want to draw too
* @param x The x coordinate to draw too
* @param y The y coordinate to draw too
*/
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y) {
	//Setup the destination rectangle to be at the position we want
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	//Query the texture to get its width and height to use
	SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
	SDL_RenderCopy(ren, tex, NULL, &dst);
}


int main(int, char**) {

	int ximage = 0;
	int yimage = 0;
	int xletras = 100;
	int yletras = 300;
	int speedx, speedy;
	string ximagedirection = "right";
	string yimagedirection = "down";
	srand(time(NULL));

	//Start up SDL and make sure it went ok
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		logSDLError( cout, "SDL_Init");
		return 1;
	}

	//Setup our window and renderer
	SDL_Window *window = SDL_CreateWindow("Lesson 2", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == nullptr) {
		logSDLError( cout, "CreateWindow");
		SDL_Quit();
		return 1;
	}
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr) {
		logSDLError( cout, "CreateRenderer");
		cleanup(window);
		SDL_Quit();
		return 1;
	}

	//The textures we'll be using
	SDL_Texture *background = loadTexture("C:/Users/juan pablo/Documents/Visual Studio 2017/Projects/sdl2test/sdl2test/background.bmp", renderer);
	SDL_Texture *image = loadTexture("C:/Users/juan pablo/Documents/Visual Studio 2017/Projects/sdl2test/sdl2test/image.bmp", renderer);
	SDL_Texture *ju = loadTexture("C:/Users/juan pablo/Documents/Visual Studio 2017/Projects/sdl2test/sdl2test/ju.bmp", renderer);
	SDL_Texture *an = loadTexture("C:/Users/juan pablo/Documents/Visual Studio 2017/Projects/sdl2test/sdl2test/an.bmp", renderer);
	SDL_Texture *pi = loadTexture("C:/Users/juan pablo/Documents/Visual Studio 2017/Projects/sdl2test/sdl2test/pi.bmp", renderer);
	
	//Make sure they both loaded ok
	if (background == nullptr || image == nullptr || ju == nullptr || an == nullptr || pi == nullptr) {
		cleanup(background, image, renderer, window);
		SDL_Quit();
		return 1;
	}

	for (int i = 0; i < 100000; ++i) {
		//Clear the window
		SDL_RenderClear(renderer);

		//Get the width and height from the texture so we know how much to move x,y by
		//to tile it correctly
		int bW, bH;
		SDL_QueryTexture(background, NULL, NULL, &bW, &bH);
		//We want to tile our background so draw it 4 times
		renderTexture(background, renderer, 0, 0);
		renderTexture(background, renderer, bW, 0);
		renderTexture(background, renderer, 2 * bW, 0);
		renderTexture(background, renderer, 3 * bW, 0);
		renderTexture(background, renderer, 0, bH);
		renderTexture(background, renderer, bW, bH);
		renderTexture(background, renderer, 2 * bW, bH);
		renderTexture(background, renderer, 3 * bW, bH);
		renderTexture(background, renderer, 0, 2* bH);
		renderTexture(background, renderer, bW, 2* bH);
		renderTexture(background, renderer, 2 * bW, 2* bH);
		renderTexture(background, renderer, 3 * bW, 2* bH);

		if (ximage >= 1080) {
			ximagedirection = "left";
			speedx = (rand() % 9 + 1);
		}
		if (ximage <= 2) {
			ximagedirection = "right";
			speedx = (rand() % 9 + 1);
		}
		if (yimage <= 2) {
			yimagedirection = "down";
			speedy = (rand() % 9 + 1);
		}
		if (yimage >= 520) {
			yimagedirection = "up";
			speedy = (rand() % 9 + 1);
		}
		if (ximagedirection == "right") {
			ximage = ximage + speedx;
		}
		if (ximagedirection == "left") {
			ximage = ximage - speedx;
		}
		if (yimagedirection == "down") {
			yimage = yimage + speedy;
		}
		if (yimagedirection == "up") {
			yimage = yimage - speedy;
		}

		renderTexture(ju, renderer, xletras, yletras);
		renderTexture(an, renderer, xletras + 370, yletras);
		renderTexture(pi, renderer, xletras + 370 + 370, yletras);
		renderTexture(image, renderer, ximage, yimage);
		//Update the screen
		SDL_RenderPresent(renderer);

		SDL_Delay(10);
	}

	cleanup(background, image, renderer, window);
	SDL_Quit();

	return 0;
}