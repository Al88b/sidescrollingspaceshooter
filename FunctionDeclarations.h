#ifndef FUNCTIONDECLARATIONS_H
#define FUNCTIONDECLARATIONS_H

#include "Laser.h"
#include "Explosion.h"
#include "Debris.h"
#include <SDL2/SDL_mixer.h>

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
#define MAX_SND_CHANNELS 8
#define MAX_LINE_LENGTH 1024
#define GLYPH_HEIGHT 28
#define GLYPH_WIDTH  18
#define NUM_HIGHSCORES 8
#define STRNCPY(dest, src, n) \
	strncpy(dest, src, n);    \
	dest[n - 1] = '\0'
#define MAX_NAME_LENGTH 32
#define MAX_SCORE_NAME_LENGTH 16
#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720

enum
{
CH_ANY = -1,
CH_PLAYER,
CH_ALIEN_FIRE,
CH_POINTS
};

enum
{
SND_PLAYER_FIRE,
SND_ALIEN_FIRE,
SND_PLAYER_DIE,
SND_ALIEN_DIE,
SND_POINTS,
SND_MAX
};

enum gameState
{
HIGHSCORESCREEN,
GAMESCREEN,
ENTERHIGHSCORESCREEN
};

enum gameState myGameState;

enum
{
	TEXT_LEFT,
	TEXT_CENTER,
	TEXT_RIGHT
};


const char* format[] = { "TEXT_LEFT", "TEXT_CENTER", "TEXT_RIGHT" };


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

void blitRect(SDL_Texture *texture, SDL_Rect *src, int x, int y, bool right);

static void doExplosions(void);

static void doDebris(void);

static void addExplosions(int x, int y, int num);

static void addDebris(Laser *e);

static void drawDebris(void);

static void drawExplosions(void);

void blit(SDL_Texture* texture, int x, int y);

void initSounds(void);
void loadMusic(char *filename);
void playMusic(int loop);
void playSound(int id, int channel);

static void loadSounds(void);

static Mix_Music *music;

static Mix_Chunk *sounds[SND_MAX];

void initFonts(void);

void drawText(int x, int y, int r, int g, int b, char *format, ...);

static void drawHud(void);

static void doPointsPods(void);

static void drawPointsPods(void);

static void addPointsPod(int x, int y);

static void highScoreComparator(const void *a, const void *b);

void initGame(void);

SDL_Texture *loadTexture(char *filename);

void initHighScores(void);

void initHighScoreTable(void);

static void drawHighscores(void);

void addHighscore(int score);

static int highscoreComparator(const void *a, const void *b);

static void drawHighScore(void);

static SDL_Texture *getTexture(char *name);

static void addTextureToCache(char *name, SDL_Texture *sdlTexture);

void drawScores(void);

static void drawNameInput(void);

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

#endif