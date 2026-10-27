
#include "Dot.cpp"
#include "Laser.h"
#include "Stage.h"
#include "Explosion.h"
#include "Debris.h"
#include "Highscores.h"
#include "Texture.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <stdbool.h>
#include <cstdlib>
#include <cctype>

Dot dot;
Laser laser;
Stage stage;
Highscores highscores;

static Laser      *player;
static SDL_Texture *bulletTexture;
static SDL_Texture *alienBulletTexture;
static SDL_Texture *enemyTexture;
static SDL_Texture *charlesTexture;
static SDL_Texture *currentEnemyTexture;
static SDL_Texture *playerTexture;
static SDL_Texture *explosionTexture;
static SDL_Texture *fontTexture;
static SDL_Texture *pointsTexture;
static SDL_Texture *sdl2Texture;
static SDL_Texture *shooterTexture;

static char         drawTextBuffer[MAX_LINE_LENGTH];
static int          enemySpawnTimer;
static int	    stageResetTimer;
static int          highscore;
static Highscore *newHighscore;
static int        cursorBlink;
static int          reveal = 0;
static int        timeout;

static Uint32 gameStartTicks = 0;

int lives = 5;

bool right;

bool makeEnemies = false;

// ---- Boss ----
#define BOSS_HEALTH     30   // player hits needed to destroy it
#define BOSS_SPEED_X    3
#define BOSS_SPEED_Y    4
#define BOSS_SPAWN_MS   35000

// ---- Wave timings (ms since the game started) ----
#define WAVE1_START_MS  5000
#define WAVE1_END_MS    15000
#define WAVE2_START_MS  (WAVE1_END_MS + 5000)    // 20s: Charles wave
#define WAVE2_END_MS    (WAVE2_START_MS + 10000) // 30s
// Boss arrives 5s after wave 2 ends -> BOSS_SPAWN_MS above (35s)

static int winTimer = 0;
static int fireLock = 0;
static void winGame(void);

static Laser       *boss = NULL;
static SDL_Texture *bossTexture;
static bool         bossSpawned = false;
static bool         bossEntering = false;
static int          bossTurnTimer = 0;

static void spawnBoss(void);
static void doBoss(void);
static void drawBoss(void);
static int  bulletHitBoss(Laser *b);

bool init()
{
//Initialization flag
bool success = true;

//Initialize SDL
if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
{
	printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
	success = false;
}
else
{
	openController();

	//Set texture filtering to linear
	if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
	{
		printf("Warning: Linear texture filtering not enabled!");
	}

	//Create window
	gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if(gWindow == NULL)
	{
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Create vsynced renderer for window
		gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if(gRenderer == NULL)
		{
			printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Initialize renderer color
			SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

			//Initialize PNG loading
			int imgFlags = IMG_INIT_PNG;
			if(!(IMG_Init(imgFlags) & imgFlags))
			{
				printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
				success = false;
			}
		}
	}
}

if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
{
	printf("Couldn't initialize SDL Mixer\n");
exit(1);
}
Mix_AllocateChannels(MAX_SND_CHANNELS);

return success;

}

void initSounds(void)
{
memset(sounds, 0, sizeof(Mix_Chunk*) * SND_MAX);
music = NULL;

loadSounds();
}

static void loadSounds(void)
{
sounds[SND_PLAYER_FIRE] = Mix_LoadWAV("334227__jradcoolness__laser.ogg");
sounds[SND_ALIEN_FIRE] = Mix_LoadWAV("196914__dpoggioli__lasergun.ogg");
sounds[SND_PLAYER_DIE] = Mix_LoadWAV("245372__quaker540__hq-explosion.ogg");
sounds[SND_ALIEN_DIE] = Mix_LoadWAV("10 Guage Shotgun-SoundBible.com-74120584.ogg");
sounds[SND_POINTS] = Mix_LoadWAV("powerUpSound.ogg");
}

void loadMusic(char *filename)
{
if(music != NULL)
{
Mix_HaltMusic();
Mix_FreeMusic(music);
music = NULL;
}

music = Mix_LoadMUS(filename);
}

void playMusic(int loop)
{
Mix_PlayMusic(music, (loop) ? -1 : 0);
}

void playSound(int id, int channel)
{
Mix_PlayChannel(channel, sounds[id], 0);
}

bool loadMedia()
{
//Loading success flag
bool success = true;

//Load background texture
if(!gBGTexture.loadFromFile("space.png"))
{
	printf("Failed to load background texture!\n");
	success = false;
}

return success;

}

void close()
{
//Free loaded images
gDotTexture.free();
gBGTexture.free();

delete player;
//delete bulletTexture;

//Destroy window	
SDL_DestroyRenderer(gRenderer);
SDL_DestroyWindow(gWindow);
gWindow = NULL;
gRenderer = NULL;

closeController();

//Quit SDL subsystems
IMG_Quit();
SDL_Quit();

}

void doKeyUp(SDL_KeyboardEvent* e)
{
if(e->repeat == 0 && e->keysym.scancode < MAX_KEYBOARD_KEYS)
{
dot.keyboard[e->keysym.scancode] = 0;
}

}

void doKeyDown(SDL_KeyboardEvent* e)
{
if(e->repeat == 0 && e->keysym.scancode < MAX_KEYBOARD_KEYS)
{
dot.keyboard[e->keysym.scancode] = 1;
}

}

void blit(SDL_Texture* texture, int x, int y)
{

//std::cout<<"Blit"<<std::endl;
SDL_Rect dest;

dest.x = x;
dest.y = y;
SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);

SDL_RenderCopy(gRenderer, texture, NULL, &dest);

}

void blitAlienBullets(SDL_Texture* texture, int x, int y)
{

//std::cout<<"Blit"<<std::endl;
SDL_Rect dest;

dest.x = x;
dest.y = y;
SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);

if(right==false)
{
SDL_RenderCopyEx(gRenderer, texture, NULL, &dest, 0.0, NULL, SDL_FLIP_NONE);
}
else
{
SDL_RenderCopyEx(gRenderer, texture, NULL, &dest, 0.0, NULL, SDL_FLIP_HORIZONTAL);
}
}

SDL_Texture* loadTexture(const char* filename)
{
SDL_Texture* texture;

//SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);

texture = IMG_LoadTexture(gRenderer, filename);

return texture;

}

void initStage(void)
{
    dot.delegate.logic = logic;
    dot.delegate.draw = draw;

    stage.explosionTail = &stage.explosionHead;
    stage.debrisTail = &stage.debrisHead;
    stage.fighterTail = &stage.fighterHead;
    stage.bulletTail = &stage.bulletHead;

    bulletTexture = loadTexture("playerBullet.png");

    enemyTexture = loadTexture("enemy.png");

    charlesTexture = loadTexture("charles.png");

    currentEnemyTexture = enemyTexture;

    alienBulletTexture = loadTexture("alienBullet.png");

    playerTexture = loadTexture("player.png");
	
    explosionTexture = loadTexture("explosion.png");

    pointsTexture = loadTexture("points.png");

    bossTexture = loadTexture("boss.png");

    resetStage();
}

static void logic(void)
{
//std::cout<<"logic"<<std::endl;
	if(winTimer > 0)
	{
		doExplosions();
		doDebris();
		if(--winTimer <= 0)
		winGame();
		return;
	}	

	doPlayer();
	
	doEnemies();

	doFighters();

	doBoss();

	doBullets();

	if(makeEnemies==true)
	{
	spawnEnemies();
	}

	clipPlayer();

	if (player == NULL && --stageResetTimer <= 0)
	{
		resetStage();
		
		//addHighscore(stage.score);

		//initHighScores();
	}

	doExplosions();

	doDebris();

	doPointsPods();
	
}

void highscoreLogic(void)
{

	if(++cursorBlink >= FPS)
	{
	cursorBlink = 0;
	}
}

void enterHighscoreLogic(void)
{
	doNameInput();

	if(++cursorBlink >= FPS)
	{
	cursorBlink = 0;
	}
}

void logicTitle(void)
{

if(reveal < SCREEN_HEIGHT)
{
reveal++;
}

if(--timeout<=0)
{
myGameState=HIGHSCORESCREEN;
timeout = FPS * 5;
}

if(fireLock > 0)
{
fireLock--;
}
else if((inputFire()) || (dot.keyboard[SDL_SCANCODE_LCTRL]))
{
startNewGame();
}
}

void logicScores(void)
{

if(--timeout<=0)
{
myGameState=TITLESCREEN;
timeout = FPS * 5;
reveal = 0;
}

}

static void initPlayer()
{
//std::cout<<"initPlayer!"<<std::endl;
    player = new Laser();
    player->health = 1;
    stage.fighterTail->next = player;
    player->next = NULL;
    stage.fighterTail = player;

    player->x = 100;
    player->y = 100;
    player->texture = loadTexture("player.png");
    SDL_QueryTexture(player->texture, NULL, NULL, &player->w, &player->h);
    player->side = SIDE_PLAYER;
}

static void doFighters(void)
{
	Laser *e, *prev;

	prev = &stage.fighterHead;

	for (e = stage.fighterHead.next; e != NULL; e = e->next)
	{
		e->x += e->dx;
		e->y += e->dy;

		if (e != player && e->x < -e->w)
		{
			e->health = 0;
		}

		if (e->health == 0)
		{
			if (e == player)
			{
				player = NULL;
			}

			if (e == stage.fighterTail)
			{
				stage.fighterTail = prev;
			}

			prev->next = e->next;
			delete e;
			e = prev;
		}

		prev = e;
	}
}


static void spawnEnemies(void)
{
	Laser *enemy;

	if (--enemySpawnTimer <= 0)
	{
		enemy = new Laser();
		stage.fighterTail->next = enemy;
		stage.fighterTail = enemy;

		enemy->x = SCREEN_WIDTH;
		enemy->y = rand() % SCREEN_HEIGHT;
		enemy->texture = currentEnemyTexture;
		SDL_QueryTexture(enemy->texture, NULL, NULL, &enemy->w, &enemy->h);

		enemy->dx = -(2 + (rand() % 4));
		enemy->dy = -100 + (rand() % 200);
		enemy->dy /= 100;

		enemy->side = SIDE_ALIEN;
		enemy->health = 1;

		enemy->reload = FPS * (1 + (rand() % 3));

		enemySpawnTimer = 30 + (rand() % FPS);
	}
}

static void drawFighters(void)
{
	
	//std::cout<<"drawFighters!"<<std::endl;
	Laser *e;

	for(e = stage.fighterHead.next ; e != NULL ; e = e->next)
	{
	blit(e->texture, e->x, e->y);
	//std::cout<<"for loop drawFighters!"<<std::endl;
	}
}

static void doPlayer(void)
{
//std::cout<<"doPlayer"<<std::endl;
	if (player != NULL)
	{
		player->dx = player->dy = 0;

		if (player->reload > 0)
		{
			player->reload--;
		}

		if (inputUp())
		{
			player->dy = -PLAYER_SPEED;
		}

		if (inputDown())
		{
			player->dy = PLAYER_SPEED;
		}

		if (inputLeft())
		{
			player->dx = -PLAYER_SPEED;
		}

		if (inputRight())
		{
			player->dx = PLAYER_SPEED;
		}

		if (inputFire() && player->reload <= 0)
		{
			fireBullet();
			playSound(SND_PLAYER_FIRE, CH_PLAYER);

		}
	}
}

static void doBullets(void)
{
	Laser *b, *prev;

	prev = &stage.bulletHead;

	for (b = stage.bulletHead.next; b != NULL; b = b->next)
	{
		b->x += b->dx;
		b->y += b->dy;

		if (bulletHitFighter(b) || bulletHitBoss(b) || b->x < -b->w || b->y < -b->h || b->x > SCREEN_WIDTH || b->y > SCREEN_HEIGHT)
		{
			if (b == stage.bulletTail)
			{
				stage.bulletTail = prev;
			}

			prev->next = b->next;
			delete b;
			b = prev;
		}

		prev = b;
	}
}

static void fireBullet(void)
{
//std::cout<<"fireBullet"<<std::endl;
Laser *bullet = new Laser();

stage.bulletTail->next=bullet;
stage.bulletTail=bullet;

bullet->x = player->x;
bullet->y = player->y;
bullet->dx = PLAYER_BULLET_SPEED;
bullet->health = 1;
bullet->texture = bulletTexture;
bullet->side = SIDE_PLAYER;
SDL_QueryTexture(bullet->texture, NULL, NULL, &bullet->w, &bullet->h);

bullet->y += (player->h / 2) - (bullet->h /2);

player->reload = 8;
}

static void draw(void)
{

    drawBullets();

    drawFighters();

    drawBoss();

    drawDebris();

    drawExplosions();

    drawHud();

    if(winTimer > 0)
    {
    drawText(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - GLYPH_HEIGHT / 2, 255, 255, 0, TEXT_CENTER, "WINNER!");
    int smallY = SCREEN_HEIGHT / 2 + GLYPH_HEIGHT / 2 + 8;
    int lineGap = (int)(GLYPH_HEIGHT * 0.6f) + 4;
    drawSmallText(SCREEN_WIDTH / 2, smallY, 255, 255, 255, TEXT_CENTER, 0.6f, "THE NEW WORLD ORDER HAS BEEN SMASHED");
    drawSmallText(SCREEN_WIDTH / 2, smallY + lineGap, 255, 255, 255, TEXT_CENTER, 0.6f, "PEACE LOVE AND ANARCHY WILL REIGN IN THE SOLAR SYSTEM FOR MILLENEA!"); 
    }

    drawPointsPods();
   

}

void drawScores(void)
{
if(newHighscore != NULL)
{
drawNameInput();
}
else
{
drawHighscores();
}
}

void drawTitle(void)
{
drawLogo();

if(timeout % 40 < 20)
{
drawText(SCREEN_WIDTH / 2, 600, 255, 255, 255, TEXT_CENTER, "PRESS FIRE TO PLAY!");
}

}

//static void drawPlayer(void)
//{
//    blit(player->texture, player->x, player->y);
//}

static void drawBullets(void)
{
    //std::cout<<"drawBullets"<<std::endl;
    Laser *b;

    for (b = stage.bulletHead.next ; b != NULL ; b = b->next)
    {
        blitAlienBullets(b->texture, b->x, b->y);
    }
}

static int bulletHitFighter(Laser *b)
{
//std::cout<<"bulletHitFighter!"<<std::endl;
	Laser * e;

	for(e = stage.fighterHead.next ; e != NULL ; e = e->next)
	{
		if(e->side != b->side && collision(b->x, b->y, b->w, b->h, e->x, e->y, e->w, e->h))
		{
 
			b->health = 0;
			e->health = 0;
			addExplosions(e->x,e->y,32);
			
			if(e==player)
			{
			playSound(SND_PLAYER_DIE, CH_PLAYER);

			lives--;
			std::cout<<"Lives = "<<lives<<std::endl;
			}
			else
			{
			playSound(SND_ALIEN_DIE, CH_ANY);
			addPointsPod(e->x + e->w / 2, e->y + e->h / 2);
			stage.score++;
			highscore = MAX(stage.score,highscore);
			}
			return 1;
}
	}
return 0;
}


void prepareScene(void)
{
	SDL_SetRenderDrawColor(dot.renderer, 32, 32, 32, 255);
	SDL_RenderClear(dot.renderer);
}

void presentScene(void)
{
	SDL_RenderPresent(dot.renderer);
}

SDL_Texture *loadTexture(char *filename)
{
	SDL_Texture *texture;

	//SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);

	texture = IMG_LoadTexture(dot.renderer, filename);
if(IMG_LoadTexture == 0)
{
std::cout<<"Image not loaded!"<<std::endl;
}
else
{
std::cout<<"Image loaded!"<<std::endl;
}
	return texture;
}


void initSDL(void)
{
	int rendererFlags, windowFlags;

	rendererFlags = SDL_RENDERER_ACCELERATED;

	windowFlags = 0;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	dot.window = SDL_CreateWindow("Shooter 05", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	dot.renderer = SDL_CreateRenderer(dot.window, -1, rendererFlags);

	IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

	SDL_ShowCursor(0);
}

int collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
	return (MAX(x1, x2) < MIN(x1 + w1, x2 + w2)) && (MAX(y1, y2) < MIN(y1 + h1, y2 + h2)); 
}

void calcScope(int x1, int y1, int x2, int y2, float *dx, float *dy)
{
int steps = MAX(abs(x1 - x2), abs(y1 - y2));

if (steps == 0)
{
*dx = *dy = 0;
return;
}

*dx = (x1 - x2);
*dx /= steps;

*dy = (y1 - y2);
*dy /= steps;
}

static void resetStage(void)
{
Laser *e;
Explosion *ex;
Debris *d;

	while (stage.fighterHead.next)
	{
		e = stage.fighterHead.next;
		stage.fighterHead.next = e->next;
		delete e;
	}

	while (stage.bulletHead.next)
	{
		e = stage.bulletHead.next;
		stage.bulletHead.next = e->next;
		delete e;
	}



	while (stage.explosionHead.next)
	{
	ex = stage.explosionHead.next;
	stage.explosionHead.next = ex->next;
	delete ex;
	}

	while(stage.debrisHead.next)
	{
	d = stage.debrisHead.next;
	stage.debrisHead.next = d->next;
	delete d;
	}

	while(stage.pointsHead.next)
	{
	e = stage.pointsHead.next;
	stage.pointsHead.next = e->next;
	delete e;
	}

	stage.fighterTail = &stage.fighterHead;
	stage.bulletTail = &stage.bulletHead;
        stage.explosionTail = &stage.explosionHead;
	stage.debrisTail = &stage.debrisHead;
	stage.pointsTail = &stage.pointsHead;


	
	if (boss != NULL)
	{
		delete boss;
		boss = NULL;
	}
	bossSpawned = false;

	initPlayer();

	enemySpawnTimer = 0;

	stageResetTimer = FPS * 2;

	gameStartTicks = 0;

	winTimer = 0;

}

static void doEnemies(void)
{
	Laser *e;

	for (e = stage.fighterHead.next; e != NULL; e = e->next)
	{
		if (e != player)
		{
			e->y = MIN(MAX(e->y, 0), SCREEN_HEIGHT - e->h);

			if (player != NULL && --e->reload <= 0)
			{
				fireAlienBullet(e);

				playSound(SND_ALIEN_FIRE, CH_ALIEN_FIRE);
			}
		}
	}
}


static void fireAlienBullet(Laser *e)
{
	Laser *bullet;

	bullet = new Laser();
	stage.bulletTail->next = bullet;
	stage.bulletTail = bullet;

	bullet->x = e->x;
	bullet->y = e->y;
	bullet->health = 1;
	bullet->texture = alienBulletTexture;
	bullet->side = SIDE_ALIEN;
	SDL_QueryTexture(bullet->texture, NULL, NULL, &bullet->w, &bullet->h);

	bullet->x += (e->w / 2) - (bullet->w / 2);
	bullet->y += (e->h / 2) - (bullet->h / 2);

	calcSlope(player->x + (player->w / 2), player->y + (player->h / 2), e->x, e->y, &bullet->dx, &bullet->dy);

	bullet->dx *= ALIEN_BULLET_SPEED;
	bullet->dy *= ALIEN_BULLET_SPEED;

	e->reload = (rand() % FPS * 2);

	if(bullet->dx>=1)
	{
	right=true;
	}
	else
	{
	right=false;
	}
}

static void clipPlayer(void)
{
	if (player != NULL)
	{
		if (player->x < 0)
		{
			player->x = 0;
		}

		if (player->y < 0)
		{
			player->y = 0;
		}

		if (player->x > SCREEN_WIDTH / 2)
		{
			player->x = SCREEN_WIDTH / 2;
		}

		if (player->y > SCREEN_HEIGHT - player->h)
		{
			player->y = SCREEN_HEIGHT - player->h;
		}
	}
}


void calcSlope(int x1, int y1, int x2, int y2, float *dx, float *dy)
{
	int steps = MAX(abs(x1 - x2), abs(y1 - y2));

	if (steps == 0)
	{
		*dx = *dy = 0;
		return;
	}

	*dx = (x1 - x2);
	*dx /= steps;

	*dy = (y1 - y2);
	*dy /= steps;
}

void blitRect(SDL_Texture *texture, SDL_Rect *src, int x, int y)
{
SDL_Rect dest;

dest.x = x;
dest.y = y;
dest.w = src->w;
dest.h = src->h;

SDL_RenderCopy(gRenderer, texture, src, &dest);

}
static void doExplosions(void)
{
Explosion *e, *prev;

prev = &stage.explosionHead;

for (e = stage.explosionHead.next ; e != NULL ; e = e->next)
{
	e->x += e->dx;
	e->y += e->dy;

	if(--e->a <= 0)
	{
		if(e == stage.explosionTail)
		{
		stage.explosionTail = prev;
		}

	prev->next = e->next;
	delete e;
	e = prev;
}

prev = e;
}
}

static void doDebris(void)
{
Debris *d, *prev;

prev = &stage.debrisHead;

for(d = stage.debrisHead.next ; d != NULL; d = d->next)
{
	d->x += d->dx;
	d->y += d->dy;

	d->dy += 0.5;

if (--d->life <= 0)
{
	if(d == stage.debrisTail)
	{
		stage.debrisTail = prev;
	}

	prev->next = d->next;
	delete d;
	d = prev;
}
prev = d;
}
}

static void addExplosions(int x, int y, int num)
{
	Explosion *e;
	int        i;

	for (i = 0; i < num; i++)
	{
		e = new Explosion();
		stage.explosionTail->next = e;
		stage.explosionTail = e;

		e->x = x + (rand() % 32) - (rand() % 32);
		e->y = y + (rand() % 32) - (rand() % 32);
		e->dx = (rand() % 10) - (rand() % 10);
		e->dy = (rand() % 10) - (rand() % 10);

		e->dx /= 10;
		e->dy /= 10;

		switch (rand() % 4)
		{
			case 0:
				e->r = 255;
				break;

			case 1:
				e->r = 255;
				e->g = 128;
				break;

			case 2:
				e->r = 255;
				e->g = 255;
				break;

			default:
				e->r = 255;
				e->g = 255;
				e->b = 255;
				break;
		}

		e->a = rand() % FPS * 3;
	}
}

static void addDebris(Laser *e)
{
	Debris *d;
	int     x, y, w, h;

	w = e->w / 2;
	h = e->h / 2;

	for (y = 0; y <= h; y += h)
	{
		for (x = 0; x <= w; x += w)
		{
			d = new Debris();
			stage.debrisTail->next = d;
			stage.debrisTail = d;

			d->x = e->x + e->w / 2;
			d->y = e->y + e->h / 2;
			d->dx = (rand() % 5) - (rand() % 5);
			d->dy = -(5 + (rand() % 12));
			d->life = FPS * 2;
			d->texture = e->texture;

			d->rect.x = x;
			d->rect.y = y;
			d->rect.w = w;
			d->rect.h = h;
		}
	}
}

static void drawDebris(void)
{
	Debris *d;

	for (d = stage.debrisHead.next; d != NULL; d = d->next)
	{
		blitRect(d->texture, &d->rect, d->x, d->y);
	}
}

static void drawExplosions(void)
{
	Explosion *e;

	SDL_SetRenderDrawBlendMode(dot.renderer, SDL_BLENDMODE_ADD);
	SDL_SetTextureBlendMode(explosionTexture, SDL_BLENDMODE_ADD);

	for (e = stage.explosionHead.next; e != NULL; e = e->next)
	{
		SDL_SetTextureColorMod(explosionTexture, e->r, e->g, e->b);
		SDL_SetTextureAlphaMod(explosionTexture, e->a);

		blit(explosionTexture, e->x, e->y);
	}

	SDL_SetRenderDrawBlendMode(dot.renderer, SDL_BLENDMODE_NONE);
}

void initFonts(void)
{
fontTexture = loadTexture("font.png");
}

void drawText(int x, int y, int r, int g, int b, int align, char *format, ...)
{
	int      i, len, c;
	SDL_Rect rect;
	va_list  args;

	va_start(args, format);
	vsprintf(drawTextBuffer, format, args);
	va_end(args);

	len = strlen(drawTextBuffer);

	switch (align)
	{
		case TEXT_RIGHT:
			x -= (len * GLYPH_WIDTH);
			break;

		case TEXT_CENTER:
			x -= (len * GLYPH_WIDTH) / 2;
			break;
	}

	rect.w = GLYPH_WIDTH;
	rect.h = GLYPH_HEIGHT;
	rect.y = 0;

	SDL_SetTextureColorMod(fontTexture, r, g, b);

	for (i = 0; i < len; i++)
	{
		c = drawTextBuffer[i];

		if (c >= ' ' && c <= 'Z')
		{
			rect.x = (c - ' ') * GLYPH_WIDTH;

			blitRect(fontTexture, &rect, x, y);

			x += GLYPH_WIDTH;
		}
	}
}

void drawSmallText(int x, int y, int r, int g, int b, int align, float scale, const char *text)
{
	int w = (int)(GLYPH_WIDTH * scale);
	int h = (int)(GLYPH_HEIGHT * scale);
	int len = strlen(text);

	if (align == TEXT_CENTER) x -= (len * w) / 2;
	else if (align == TEXT_RIGHT) x -= len * w;

	SDL_Rect src = {0, 0, GLYPH_WIDTH, GLYPH_HEIGHT};
	SDL_Rect dest = {x, y, w, h};

	SDL_SetTextureColorMod(fontTexture, r, g, b);

	for (int i = 0; i < len; i++)
	{
		int c = toupper((unsigned char)text[i]);
		if (c >= ' ' && c <= 'Z')
		{
			src.x = (c - ' ') * GLYPH_WIDTH;
			SDL_RenderCopy(gRenderer, fontTexture, &src, &dest);
		}
		dest.x += w;
	}
}

static void drawHud(void)
{
	drawText(10, 10, 255, 255, 255, TEXT_LEFT, "SCORE: %03d", stage.score);

	if (stage.score < highscores.highscore[0].score)
	{
		drawText(SCREEN_WIDTH - 10, 10, 255, 255, 255, TEXT_RIGHT, "HIGHSCORE: %03d", highscores.highscore[0].score);
	}
	else
	{
		drawText(SCREEN_WIDTH - 10, 10, 0, 255, 0, TEXT_RIGHT, "HIGHSCORE: %03d", stage.score);
	}
}

static void doPointsPods(void)
{
	Laser *e, *prev;

	prev = &stage.pointsHead;

	for (e = stage.pointsHead.next; e != NULL; e = e->next)
	{
		if (e->x < 0)
		{
			e->x = 0;
			e->dx = -e->dx;
		}

		if (e->x + e->w > SCREEN_WIDTH)
		{
			e->x = SCREEN_WIDTH - e->w;
			e->dx = -e->dx;
		}

		if (e->y < 0)
		{
			e->y = 0;
			e->dy = -e->dy;
		}

		if (e->y + e->h > SCREEN_HEIGHT)
		{
			e->y = SCREEN_HEIGHT - e->h;
			e->dy = -e->dy;
		}

		e->x += e->dx;
		e->y += e->dy;

		if (player != NULL && collision(e->x, e->y, e->w, e->h, player->x, player->y, player->w, player->h))
		{
			e->health = 0;

			stage.score++;

			highscore = MAX(stage.score, highscore);

			playSound(SND_POINTS, CH_POINTS);
		}

		if (--e->health <= 0)
		{
			if (e == stage.pointsTail)
			{
				stage.pointsTail = prev;
			}

			prev->next = e->next;
			delete e;
			e = prev;
		}

		prev = e;
	}
}

static void addPointsPod(int x, int y)
{
	Laser *e;

	e = new Laser();
	stage.pointsTail->next = e;
	stage.pointsTail = e;

	e->x = x;
	e->y = y;
	e->dx = -(rand() % 5);
	e->dy = (rand() % 5) - (rand() % 5);
	e->health = FPS * 10;
	e->texture = pointsTexture;

	SDL_QueryTexture(e->texture, NULL, NULL, &e->w, &e->h);

	e->x -= e->w / 2;
	e->y -= e->h / 2;
}

static void drawPointsPods(void)
{
	Laser *e;

	for (e = stage.pointsHead.next; e != NULL; e = e->next)
	{
		if (e->health > (FPS * 2) || e->health % 12 < 6)
		{
			blit(e->texture, e->x, e->y);
		}
	}
}


void initHighScores(void)
{
dot.delegate.logic = logic;
dot.delegate.draw = draw;
dot.delegate.drawScores = drawScores;
dot.delegate.logicScores = logicScores;
}

void initHighscoreTable(void)
{
	int i;

	for (i = 0; i < NUM_HIGHSCORES; i++)
	{
		highscores.highscore[i].score = NUM_HIGHSCORES - i;
		STRNCPY(highscores.highscore[i].name, "ANONYMOUS", MAX_SCORE_NAME_LENGTH);
	}

	newHighscore = NULL;

	cursorBlink = 0;
}

void initGame(void)
{
initSounds();
initFonts();
loadMusic("Mercury.ogg");
playMusic(1);
}

static void drawHighscores(void)
{
	int i, y, r, g, b;

	y = 150;

	drawText(SCREEN_WIDTH / 2, 70, 255, 255, 255, TEXT_CENTER, "HIGHSCORES");

	for (i = 0; i < NUM_HIGHSCORES; i++)
	{
		r = 255;
		g = 255;
		b = 255;

		if (highscores.highscore[i].recent)
		{
			b = 0;
		}

		drawText(SCREEN_WIDTH / 2, y, r, g, b, TEXT_CENTER, "#%d. %-15s ...... %03d", (i + 1), highscores.highscore[i].name, highscores.highscore[i].score);

		y += 50;
	}

	drawText(SCREEN_WIDTH / 2, 600, 255, 255, 255, TEXT_CENTER, "PRESS FIRE TO PLAY!");
}

void addHighscore(int score)
{
	Highscore newHighscores[NUM_HIGHSCORES + 1];
	int       i;

	memset(newHighscores, 0, sizeof(Highscore) * (NUM_HIGHSCORES + 1));

	for (i = 0; i < NUM_HIGHSCORES; i++)
	{
		newHighscores[i] = highscores.highscore[i];
		newHighscores[i].recent = 0;
	}

	newHighscores[NUM_HIGHSCORES].score = score;
	newHighscores[NUM_HIGHSCORES].recent = 1;

	qsort(newHighscores, NUM_HIGHSCORES + 1, sizeof(Highscore), highScoreComparator);

	newHighscore = NULL;

	for (i = 0; i < NUM_HIGHSCORES; i++)
	{
		highscores.highscore[i] = newHighscores[i];

		if (highscores.highscore[i].recent)
		{
			newHighscore = &highscores.highscore[i];
		}
	}
}

static int highScoreComparator(const void *a, const void *b)
{
    Highscore *h1 = ((Highscore*)a);
    Highscore *h2 = ((Highscore*)b);

    return h2->score - h1->score;
}

static SDL_Texture *getTexture(char *name)
{
    Texture *t;

    for (t = dot.textureHead.next ; t != NULL ; t = t->next)
    {
        if (strcmp(t->name, name) == 0)
        {
            return t->texture;
        }
    }

    return NULL;
}

static void addTextureToCache(char *name, SDL_Texture *sdlTexture)
{
    Texture *texture;

    texture = new Texture();
    dot.textureTail->next = texture;
    dot.textureTail = texture;

    STRNCPY(texture->name, name, MAX_NAME_LENGTH);
    texture->texture = sdlTexture;
}

static void doNameInput(void)
{
std::cout<<"In doNameInput() "<<std::endl;
	int  i, n;
	char c;

	n = strlen(newHighscore->name);

	for (i = 0; i < strlen(dot.inputText); i++)
	{
		c = toupper(dot.inputText[i]);

		if (n < MAX_SCORE_NAME_LENGTH - 1 && c >= ' ' && c <= 'Z')
		{
			newHighscore->name[n++] = c;
		}
	}

	if (n > 0 && dot.keyboard[SDL_SCANCODE_BACKSPACE])
	{
		newHighscore->name[--n] = '\0';

		dot.keyboard[SDL_SCANCODE_BACKSPACE] = 0;
	}

	if (dot.keyboard[SDL_SCANCODE_RETURN] || inputConfirm())
	{
		if (strlen(newHighscore->name) == 0)
		{
			STRNCPY(newHighscore->name, "ANONYMOUS", MAX_SCORE_NAME_LENGTH);
		}

		newHighscore = NULL;
	}
}

static void drawNameInput(void)
{
	SDL_Rect r;

	drawText(SCREEN_WIDTH / 2, 70, 255, 255, 255, TEXT_CENTER, "CONGRATULATIONS, YOU'VE GAINED A HIGHSCORE!");

	drawText(SCREEN_WIDTH / 2, 120, 255, 255, 255, TEXT_CENTER, "ENTER YOUR NAME BELOW:");

	drawText(SCREEN_WIDTH / 2, 250, 128, 255, 128, TEXT_CENTER, newHighscore->name);

	if (cursorBlink < FPS / 2)
	{
		r.x = ((SCREEN_WIDTH / 2) + (strlen(newHighscore->name) * GLYPH_WIDTH) / 2) + 5;
		r.y = 250;
		r.w = GLYPH_WIDTH;
		r.h = GLYPH_HEIGHT;

		SDL_SetRenderDrawColor(dot.renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(dot.renderer, &r);
	}

	drawText(SCREEN_WIDTH / 2, 625, 255, 255, 255, TEXT_CENTER, "PRESS RETURN WHEN FINISHED");
}

void initTitle(void)
{
dot.delegate.logicTitle = logicTitle;
dot.delegate.drawTitle = drawTitle;

sdl2Texture = loadTexture("sdl2.png");
shooterTexture = loadTexture("shooter.png");

timeout = FPS * 5;

}

void drawLogo(void)
{
SDL_Rect r;

r.x = 0;
r.y = 0;

SDL_QueryTexture(sdl2Texture,NULL,NULL,&r.w,&r.h);

r.h = MIN(reveal,r.h);

blitRect(sdl2Texture,&r,(SCREEN_WIDTH/2)-(r.w/2),100);

SDL_QueryTexture(shooterTexture,NULL,NULL,&r.w,&r.h);

blitRect(shooterTexture,&r,(SCREEN_WIDTH/2)-(r.w/2),250);
}

void runEnemies(void)
{
if(gameStartTicks == 0)
   gameStartTicks = SDL_GetTicks();

Uint32 elapsed = SDL_GetTicks() - gameStartTicks;


bool wave1 = (elapsed >= WAVE1_START_MS && elapsed < WAVE1_END_MS);
bool wave2 = (elapsed >= WAVE2_START_MS && elapsed < WAVE2_END_MS);

makeEnemies = wave1 || wave2;

// Wave 1 uses enemy.png, wave 2 uses charles.png. Both fire the same
// alien bullets because doEnemies() handles every fighter in the list.
currentEnemyTexture = wave2 ? charlesTexture : enemyTexture;

if(elapsed >= BOSS_SPAWN_MS && !bossSpawned)
{
	spawnBoss();
	bossSpawned = true;
}

// Clear leftover enemies in the gaps after each wave
if(!makeEnemies && elapsed >= WAVE1_END_MS)
{
	for(Laser *e = stage.fighterHead.next ; e != NULL ; e = e->next)
	{
	if(e != player)
		e->health = 0;
	}
}
}

static void capFrameRate(long *then, float *remainder)
{
	long wait, frameTime;

	wait = 16 + *remainder;

	*remainder -= (int)*remainder;

	frameTime = SDL_GetTicks() - *then;

	wait -= frameTime;

	if (wait < 1)
	{
		wait = 1;
	}

	SDL_Delay(wait);

	*remainder += 0.667;

	*then = SDL_GetTicks();
}

void cleanup(void)
{
	IMG_Quit();

	SDL_DestroyRenderer(dot.renderer);

	SDL_DestroyWindow(dot.window);

	SDL_Quit();
}

// ===================== BOSS =====================

static void spawnBoss(void)
{
	boss = new Laser();
	boss->texture = bossTexture;
	SDL_QueryTexture(boss->texture, NULL, NULL, &boss->w, &boss->h);

	// Start just off the right edge, vertically centred
	boss->x = SCREEN_WIDTH;
	boss->y = (SCREEN_HEIGHT / 2) - (boss->h / 2);

	// Fly in from the right first
	boss->dx = -BOSS_SPEED_X;
	boss->dy = 0;
	bossEntering = true;

	boss->health = BOSS_HEALTH;
	boss->side = SIDE_ALIEN;
	boss->reload = FPS * 2;
	boss->next = NULL;
}

static void doBoss(void)
{
	if (boss == NULL)
		return;

	boss->x += boss->dx;
	boss->y += boss->dy;

	// The strafing area: right half of the screen
	int minX = SCREEN_WIDTH / 2;
	int maxX = SCREEN_WIDTH - boss->w;
	int minY = 0;
	int maxY = SCREEN_HEIGHT - boss->h;

	if (bossEntering)
	{
		// Once fully on screen, start strafing
		if (boss->x <= maxX - 50)
		{
			bossEntering = false;
			boss->dx = (rand() % 2) ? BOSS_SPEED_X : -BOSS_SPEED_X;
			boss->dy = (rand() % 2) ? BOSS_SPEED_Y : -BOSS_SPEED_Y;
			bossTurnTimer = FPS + rand() % (FPS * 2);
		}
		return;
	}

	// Bounce off the edges of the strafing area (left/right and up/down)
	if (boss->x < minX) { boss->x = minX; boss->dx = BOSS_SPEED_X; }
	if (boss->x > maxX) { boss->x = maxX; boss->dx = -BOSS_SPEED_X; }
	if (boss->y < minY) { boss->y = minY; boss->dy = BOSS_SPEED_Y; }
	if (boss->y > maxY) { boss->y = maxY; boss->dy = -BOSS_SPEED_Y; }

	// Every 1-3 seconds randomly flip direction so it's less predictable
	if (--bossTurnTimer <= 0)
	{
		if (rand() % 2) boss->dx = -boss->dx;
		if (rand() % 2) boss->dy = -boss->dy;
		bossTurnTimer = FPS + rand() % (FPS * 2);
	}

	// Shoot at the player (delete this block if you don't want it to fire)
	if (player != NULL && --boss->reload <= 0)
	{
		fireAlienBullet(boss);
		playSound(SND_ALIEN_FIRE, CH_ALIEN_FIRE);
		boss->reload = FPS / 2 + rand() % FPS;
	}

	// Ramming the boss kills the player
	if (player != NULL && player->health > 0 &&
	    collision(player->x, player->y, player->w, player->h,
	              boss->x, boss->y, boss->w, boss->h))
	{
		player->health = 0;
		addExplosions(player->x, player->y, 32);
		playSound(SND_PLAYER_DIE, CH_PLAYER);
		lives--;
	}
}

static int bulletHitBoss(Laser *b)
{
	if (boss == NULL || b->side != SIDE_PLAYER)
		return 0;

	if (!collision(b->x, b->y, b->w, b->h, boss->x, boss->y, boss->w, boss->h))
		return 0;

	b->health = 0;
	boss->health--;
	addExplosions(b->x, b->y, 4);   // small "hit" spark

	if (boss->health <= 0)
	{
		addExplosions(boss->x + boss->w / 2, boss->y + boss->h / 2, 96);
		playSound(SND_ALIEN_DIE, CH_ANY);
		addPointsPod(boss->x + boss->w / 2, boss->y + boss->h / 2);
		stage.score += 10;
		highscore = MAX(stage.score, highscore);

		delete boss;
		boss = NULL;
		winTimer = FPS * 10;
	}

	return 1;
}

static void drawBoss(void)
{
	if (boss == NULL)
		return;

	blit(boss->texture, boss->x, boss->y);

	// Health bar above the boss
	SDL_Rect back = { (int)boss->x, (int)boss->y - 12, boss->w, 6 };
	SDL_Rect front = back;
	front.w = boss->w * boss->health / BOSS_HEALTH;

	SDL_SetRenderDrawColor(gRenderer, 80, 0, 0, 255);
	SDL_RenderFillRect(gRenderer, &back);
	SDL_SetRenderDrawColor(gRenderer, 255, 40, 40, 255);
	SDL_RenderFillRect(gRenderer, &front);
}


// ===================== JOYPAD =====================
// Each input function returns true if EITHER the keyboard OR the
// controller is pressing it, so both can be used at the same time.

static SDL_GameController *controller = NULL;

void openController(void)
{
	if (controller != NULL)
		return;

	for (int i = 0; i < SDL_NumJoysticks(); i++)
	{
		if (SDL_IsGameController(i))
		{
			controller = SDL_GameControllerOpen(i);
			if (controller != NULL)
			{
				printf("Controller connected: %s\n", SDL_GameControllerName(controller));
				return;
			}
		}
	}
}

void closeController(void)
{
	if (controller != NULL)
	{
		SDL_GameControllerClose(controller);
		controller = NULL;
	}
}

// Lets you plug in / unplug a pad while the game is running
void handleControllerEvent(SDL_Event& e)
{
	if (e.type == SDL_CONTROLLERDEVICEADDED)
	{
		openController();
	}
	else if (e.type == SDL_CONTROLLERDEVICEREMOVED && controller != NULL)
	{
		SDL_Joystick *joy = SDL_GameControllerGetJoystick(controller);
		if (SDL_JoystickInstanceID(joy) == e.cdevice.which)
		{
			closeController();
			openController();   // switch to another pad if one is plugged in
		}
	}
}

static bool padButton(SDL_GameControllerButton b)
{
	return controller != NULL && SDL_GameControllerGetButton(controller, b);
}

static int padAxis(SDL_GameControllerAxis a)
{
	return controller != NULL ? SDL_GameControllerGetAxis(controller, a) : 0;
}

bool inputUp(void)
{
	return dot.keyboard[SDL_SCANCODE_UP]
	    || padButton(SDL_CONTROLLER_BUTTON_DPAD_UP)
	    || padAxis(SDL_CONTROLLER_AXIS_LEFTY) < -JOY_DEADZONE;
}

bool inputDown(void)
{
	return dot.keyboard[SDL_SCANCODE_DOWN]
	    || padButton(SDL_CONTROLLER_BUTTON_DPAD_DOWN)
	    || padAxis(SDL_CONTROLLER_AXIS_LEFTY) > JOY_DEADZONE;
}

bool inputLeft(void)
{
	return dot.keyboard[SDL_SCANCODE_LEFT]
	    || padButton(SDL_CONTROLLER_BUTTON_DPAD_LEFT)
	    || padAxis(SDL_CONTROLLER_AXIS_LEFTX) < -JOY_DEADZONE;
}

bool inputRight(void)
{
	return dot.keyboard[SDL_SCANCODE_RIGHT]
	    || padButton(SDL_CONTROLLER_BUTTON_DPAD_RIGHT)
	    || padAxis(SDL_CONTROLLER_AXIS_LEFTX) > JOY_DEADZONE;
}

// Fire: Left Ctrl, or A (Xbox) / Cross (PlayStation), or right trigger
bool inputFire(void)
{
	return dot.keyboard[SDL_SCANCODE_LCTRL]
	    || padButton(SDL_CONTROLLER_BUTTON_A)
	    || padAxis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > JOY_DEADZONE;
}

// Confirm high-score name: Start button (Return still works on keyboard)
bool inputConfirm(void)
{
	return padButton(SDL_CONTROLLER_BUTTON_START);
}

static void winGame(void)
{
addHighscore(stage.score);
resetStage();

fireLock = FPS;

if(newHighscore != NULL)
{
myGameState=ENTERHIGHSCORESCREEN;
}
else
{
myGameState = TITLESCREEN;
timeout = FPS * 5;
reveal = 0;
}

}

void startNewGame(void)
{
lives = 5;
stage.score = 0;
resetStage();
myGameState = GAMESCREEN;
}
