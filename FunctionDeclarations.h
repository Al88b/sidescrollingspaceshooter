#ifndef FUNCTIONDECLARATIONS_H
#define FUNCTIONDECLARATIONS_H

#include "Laser.h"

// Constants and definitions
#define PLAYER_SPEED		4
#define PLAYER_BULLET_SPEED	16
#define SIDE_PLAYER 0
#define SIDE_ALIEN 1    
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define FPS 60
#define ALIEN_BULLET_SPEED 8
#define MAX_KEYBOARD_KEYS	350

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Scene textures
LTexture gDotTexture;
LTexture gLazerTexture;
LTexture gBGTexture;

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;


// Function declarations

static void doKeyUp(SDL_KeyboardEvent* event);

static void doKeyDown(SDL_KeyboardEvent* event);

void initStage(void);

static void initPlayer();

static void logic(void);

static void doPlayer(void);

static void fireBullet(void);

static void doBullets(void);

static void draw(void);

static void drawPlayer(void);

static void drawBullets(void);

int collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);

static int bulletHitFighter(Laser *b);

static void capFrameRate(long *then, float *remainder);

void initSDL(void);

static void capFrameRate(long *then, float *remainder);

void cleanup(void);

static void doFighters (void);

static void spawnEnemies(void);

static void drawFighters(void);

void calcSlope(int x1, int y1, int x2, int y2, float *dx, float *dy);

static void resetStage(void);

static void fireAlienBullet(Laser *e);

static void clipPlayer(void);

static void doEnemies(void);

void blitRect(SDL_Texture *texture, SDL_Rect *src, int x, int y);

static void doExposions(void);

static void doDebris(void);

static void addExplosions(int x, int y, int num);

static void addDebris(Laser *e);

static void drawDebris(void);

static void drawExplosions(void);

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

#endif