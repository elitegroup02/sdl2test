#include <string>
#include <iostream>
#include <windows.h>
#include <experimental/filesystem>
#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
#include <vector>
#include "tinyxml2.h"
#include "res_path.h"
#include "cleanup.h"
namespace fs = std::experimental::filesystem::v1;
using namespace std;

#ifndef XMLCheckResult
#define XMLCheckResult(a_eResult) if (a_eResult != tinyxml2::XML_SUCCESS) { printf("Error: %i\n", a_eResult); return a_eResult; }
#endif


Uint32 get_pixel32(SDL_Surface *surface, int x, int y)
{
	//Convert the pixels to 32 bit
	Uint32 *pixels = (Uint32 *)surface->pixels;

	//Get the requested pixel
	return pixels[(y * surface->w) + x];
}

void put_pixel32(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
	//Convert the pixels to 32 bit
	Uint32 *pixels = (Uint32 *)surface->pixels;

	//Set the pixel
	pixels[(y * surface->w) + x] = pixel;
}

SDL_Surface *ScaleSurface(SDL_Surface *Surface, Uint16 Width, Uint16 Height)
{
	if (!Surface || !Width || !Height) {
		return 0;
	}

	SDL_Surface *_ret = SDL_CreateRGBSurface(Surface->flags, Width, Height, Surface->format->BitsPerPixel,
		Surface->format->Rmask, Surface->format->Gmask, Surface->format->Bmask, Surface->format->Amask);

	double    _stretch_factor_x = (static_cast<double>(Width) / (static_cast<double>(Surface->w))),
		_stretch_factor_y = (static_cast<double>(Height) / static_cast<double>(Surface->h));

	for (Sint32 y = 0; y < Surface->h; y++) {
		for (Sint32 x = 0; x < Surface->w; x++) {
			for (Sint32 o_y = 0; o_y < _stretch_factor_y; ++o_y) {
				for (Sint32 o_x = 0; o_x < _stretch_factor_x; ++o_x) {
					put_pixel32(_ret, static_cast<Sint32>(_stretch_factor_x * x) + o_x,
						static_cast<Sint32>(_stretch_factor_y * y) + o_y, get_pixel32(Surface, x, y));
				}
			}
		}
	}

	SDL_SetColorKey( _ret , SDL_TRUE, SDL_MapRGB(_ret->format, 0, 0xFF, 0xFF));

	return _ret;
}

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

SDL_Surface* gScreenSurface = NULL;

SDL_Surface* loadSurface(string path)
{
	//The final optimized image
	SDL_Surface* optimizedSurface = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Convert surface to screen format
		optimizedSurface = SDL_ConvertSurface(loadedSurface, gScreenSurface->format, NULL);
		if (optimizedSurface == NULL)
		{
			printf("Unable to optimize image %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	return optimizedSurface;
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

string path(void) {

	wchar_t buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	wstring wstr(buffer);
	string str(wstr.begin(), wstr.end());

	using convert_typeX = codecvt_utf8<wchar_t>;
	wstring_convert<convert_typeX, wchar_t> converterX;

	string exepath = converterX.to_bytes(wstr);
	exepath.erase(exepath.end() - 12, exepath.end());
	return exepath;

}

void initialisefiles(string respath, vector <string>& resfiles, vector <string>& resfilesname) {
	string stringaux = "";

	for (auto & p : fs::directory_iterator(respath)) {
		resfiles.push_back(p.path().string());
	}
	for (string x : resfiles) {
		cout << x << endl;
		while (x != stringaux) {
			stringaux = x;
			x.erase(0, (x.find("\\") + 1));
		}
		cout << x << endl;
		resfilesname.push_back(x.erase(0, (x.find(".")) + 1));
	}
	for (string y : resfilesname) {
		cout << y << endl;
	}

}


int main(int, char**) {

	int resolutionx = 1280, resolutiony = 720;
	//iW and iH are the clip width and height
	//We'll be drawing only clips so get a center position for the w/h of a clip
	int iW = 100, iH = 100;
	double xPlayer = 0; //The initial x position
	double yPlayer = resolutiony - iH; //The initial height is the floor
	double Velx = 0, Vely = 0;
	double g = -0.80; //downwards accelleration, "gravity"
	bool Leftpress = 0, Rightpress = 0, Rightpressaux = 0, Leftpressaux = 0, Movingright = 0, Movingleft = 0;
	int jumpnumber = 0; //TODO: make this a thing!!!

	double xBase = 0, yBase = 0, xMountains = 0, yMountains = 0, xClouds = 0, yClouds = 0, xSun = 0, yGrass = 0;
	//TODO!! ---> Make this not like this, possibly after initializing vectors... make a vector containing x/y values or smth idk...
	double xLimit = 3.5, VelNomx = 3.5, ySun = 0, xGrass = 0;

	string opath = path();
	string respath = (opath + "res");

	cout << respath << endl;
	vector <string> resfiles;
	vector <string> resfilesname;
	vector <SDL_Texture*> opt_textures;

	initialisefiles(respath, resfiles, resfilesname);
	
	tinyxml2::XMLDocument xmlDoc;
	tinyxml2::XMLElement * pElement;
	tinyxml2::XMLError eResult = xmlDoc.LoadFile("gamesettings.xml");
	XMLCheckResult(eResult);

	if (eResult != 0)
	{
		tinyxml2::XMLNode * pRoot = xmlDoc.NewElement("Root");
		xmlDoc.InsertFirstChild(pRoot);
		pElement = xmlDoc.NewElement("Files");
		for (string x : resfilesname)
		{
			tinyxml2::XMLElement * pFileListElement = xmlDoc.NewElement("Filename");
			pFileListElement->SetText(x.c_str());
			pElement->InsertEndChild(pFileListElement);
		}
		pRoot->InsertEndChild(pElement);

		pElement = xmlDoc.NewElement("Resolution");
		tinyxml2::XMLElement * pResolutionElement = xmlDoc.NewElement("resolutionx");
		pResolutionElement->SetText(resolutionx);
		pElement->InsertEndChild(pResolutionElement);
		pResolutionElement = xmlDoc.NewElement("resolutiony");
		pResolutionElement->SetText(resolutiony);
		pElement->InsertEndChild(pResolutionElement);
		pRoot->InsertEndChild(pElement);

		tinyxml2::XMLError(xmlDoc.SaveFile("gamesettings.xml"));
	}

	tinyxml2::XMLNode * pRoot = xmlDoc.FirstChild();
	if (pRoot == nullptr) return tinyxml2::XML_ERROR_FILE_READ_ERROR;
	pElement = pRoot->FirstChildElement("Files");
	if (pElement == nullptr) return tinyxml2::XML_ERROR_PARSING_ELEMENT;
	tinyxml2::XMLElement * pListElement = pElement->FirstChildElement("Filename");
	std::vector<string> vecList;

	//YOU CAN INPUT STUFF FROM HERE ON, EVERYTHING BEFORE THIS HAPPENS BEFORE YOU TYPE ANYTHIIIING


	//Start up SDL and make sure it went ok
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		logSDLError(std::cout, "SDL_Init");
		return 1;
	}

	//Setup our window and renderer
	SDL_Window *window = SDL_CreateWindow("Moving ball of DOOooOOooO000OOOo0O0oom", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, resolutionx, resolutiony, SDL_WINDOW_SHOWN);
	if (window == nullptr) {
		logSDLError(std::cout, "CreateWindow");
		SDL_Quit();
		return 1;
	}
	gScreenSurface = SDL_GetWindowSurface(window);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr) {
		logSDLError(std::cout, "CreateRenderer");
		cleanup(window);
		SDL_Quit();
		return 1;
	}
	

	for (string x : resfiles) {
		opt_textures.push_back(SDL_CreateTextureFromSurface(renderer, ScaleSurface(loadSurface(x), resolutionx, resolutiony)));
	}

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
					Vely = - 20;
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
					Rightpressaux = 0;
					break;
				default:
					break;
				}
			}
		}

		//Movement logic goes here
		if (xPlayer <= 0) { Leftpress = 0; } //This means only the opposite button to the edge of the screen being touched can be pressed
		if (xPlayer >= (1280 - iW) / xLimit) { Rightpress = 0; } // fraction of the screen used to simulate movement. iW is image(player) width
		if (Rightpress == 1 && Leftpress == 1) //If both are pressed at the same time then stop
		{
			Velx = 0;
		}
		else //Else, only one of them is pressed. Pick the pressed side and give it a constant speed, also flag it as moving
		{
			if (Rightpress == 1) { Velx = VelNomx; Movingright = 1; }
			if (Leftpress == 1) { Velx = -VelNomx; Movingleft = 1; }
		}
		if (Movingleft == 1 && Leftpress == 0) { Velx = 0; Movingleft = 0; } //If either is pressed and the button is released, reset the speed and flag
		if (Movingright == 1 && Rightpress == 0) { Velx = 0; Movingright = 0; }

		if (yPlayer >= 720 - iH) { g = 0; } //Otherwise the player will fall off the screen in the down (y +) direction
		else { g = -0.75; } //TODO: make this modular and changeable
		Vely = Vely - g;
		xPlayer = xPlayer + Velx;
		yPlayer = yPlayer + Vely;
		if (yPlayer >= 720 - iH) { yPlayer = 720 - iH; }

		if ((xPlayer >= ((1280 - iW) / xLimit)) && Rightpressaux == 1)
		{
			xMountains = xMountains - (VelNomx) / 1.5;
			xGrass = xGrass - VelNomx;
			xClouds = xClouds - (VelNomx) / 3;
			xSun = xSun - (VelNomx) / 12;

		}

		if (xPlayer <= 0 && Leftpressaux == 1)
		{
			xMountains = xMountains + (3.5) / 1.5;
			xGrass = xGrass + 3.5;
			xClouds = xClouds + (3.5) / 3;
			xSun = xSun - (3.5) / 12;
		}

		if (xSun <= -resolutionx) { xSun = 0; }
		if (xGrass <= -resolutionx) { xGrass = 0; }
		if (xMountains <= -resolutionx) { xMountains = 0; }
		if (xClouds <= -resolutionx) { xClouds = 0; }

		//Rendering
		SDL_RenderClear(renderer);
		//Draw the image
		for (SDL_Texture* x : opt_textures) {
			renderTexture(x, renderer, xBase, yBase);
			renderTexture(x, renderer, (xBase + resolutionx), yBase);
		}
		
		SDL_RenderPresent(renderer);


	}
	//Clean up
	for (SDL_Texture* x : opt_textures) {
		cleanup(x);
	}
	// cleanup(playertextopt, suntextureopt, sunsurfaceopt, cloudssurfaceopt, cloudstextureopt, basesurfaceopt, basetextureopt, mountainssurfaceopt, mountainstextureopt, grasssurfaceopt, grasstextureopt, renderer, window);
	IMG_Quit();
	SDL_Quit();

	return 0;
}