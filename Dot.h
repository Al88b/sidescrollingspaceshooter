#ifndef DOT_H
#define DOT_H
#include "FunctionDeclarations.h"
#include "Delegate.h"
#include "Texture.h"

//The dot that will move around on the screen
class Dot
{
public:
//The dimensions of the dot
static const int DOT_WIDTH = 20;
static const int DOT_HEIGHT = 20;

//Maximum axis velocity of the dot
static const int DOT_VEL = 10;

int keyboard[MAX_KEYBOARD_KEYS];

//Initializes the variables
Dot();

//Takes key presses and adjusts the dot's velocity
void handleEvent(SDL_Event& e);

//Moves the dot
void move();

//Shows the dot on the screen
void render();

int getPosX() { return mPosX; }
int getPosY() { return mPosY; }

int fire;

SDL_Renderer* renderer;
SDL_Window* window;

SDL_Texture* texture;
	
Texture       textureHead, *textureTail;

char          inputText[MAX_LINE_LENGTH];

int x;
int y;
int dx;
int dy;

//The X and Y offsets of the dot
int mPosX, mPosY;

//The velocity of the dot
int mVelX, mVelY;

Delegate delegate;

};

#endif