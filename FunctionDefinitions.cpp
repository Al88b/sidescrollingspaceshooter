
#include "Dot.cpp"
#include "Laser.h"
#include "Stage.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
Dot dot;
Laser laser;
Stage stage;

static Laser      *player;
static SDL_Texture *bulletTexture;
static SDL_Texture *alienBulletTexture;
static SDL_Texture *enemyTexture;
static SDL_Texture *playerTexture;
static int          enemySpawnTimer;
static int	    stageResetTimer;

bool init()
{
//Initialization flag
bool success = true;

//Initialize SDL
if(SDL_Init(SDL_INIT_VIDEO) < 0)
{
	printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
	success = false;
}
else
{
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

return success;

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

//Quit SDL subsystems
IMG_Quit();
SDL_Quit();

}

void doKeyUp(SDL_KeyboardEvent* event)
{
if(event->repeat == 0 && event->keysym.scancode < MAX_KEYBOARD_KEYS)
{
dot.keyboard[event->keysym.scancode] = 0;
}

}

void doKeyDown(SDL_KeyboardEvent* event)
{
if(event->repeat == 0 && event->keysym.scancode < MAX_KEYBOARD_KEYS)
{
dot.keyboard[event->keysym.scancode] = 1;
}

}

void blit(SDL_Texture* texture, int x, int y)
{

std::cout<<"Blit"<<std::endl;
SDL_Rect dest;

dest.x = x;
dest.y = y;
SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);

SDL_RenderCopy(gRenderer, texture, NULL, &dest);

}

SDL_Texture* loadTexture(const char* filename)
{
SDL_Texture* texture;

SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);

texture = IMG_LoadTexture(gRenderer, filename);

return texture;

}

void initStage(void)
{
    dot.delegate.logic = logic;
    dot.delegate.draw = draw;

    stage.fighterTail = &stage.fighterHead;
    stage.bulletTail = &stage.bulletHead;

    bulletTexture = loadTexture("playerBullet.png");

    enemyTexture = loadTexture("enemy.png");

    alienBulletTexture = loadTexture("alienBullet.png");

    playerTexture = loadTexture("player.png");

    resetStage();

}

static void initPlayer()
{
std::cout<<"initPlayer!"<<std::endl;
    player = new Laser();
    stage.fighterTail->next = player;
    stage.fighterTail = player;

    player->x = 100;
    player->y = 100;
    player->texture = loadTexture("player.png");
    SDL_QueryTexture(player->texture, NULL, NULL, &player->w, &player->h);
    player->side = SIDE_PLAYER;
}

static void logic(void)
{
    doPlayer();

    doEnemies();

    doFighters();    

    doBullets();

    spawnEnemies();

    clipPlayer();

	if(player == NULL && --
stageResetTimer <=0)
	{
	resetStage();
	}
}

static void doFighters(void)
{
std::cout<<"doFighters!"<<std::endl;
Laser *e, *prev;

prev = &stage.fighterHead;

for(e = stage.fighterHead.next ; e != NULL ; e = e->next)

{
	e->x += e->dx;
	e->y += e->dy;

	if(e != player && e->x < -e->w)
	{
		e->health = 0;
	}
	
	if(e->health == 0)
	{
		if(e == player)
		{
		player = NULL;
		}
	

		if(e == stage.fighterTail)
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
	
	if(--enemySpawnTimer <= 0)
	{
		enemy = new Laser();
		stage.fighterTail->next = enemy;
		stage.fighterTail = enemy;

		enemy->x = SCREEN_WIDTH;
		enemy->y = rand() % SCREEN_HEIGHT;
		enemy->texture = enemyTexture;
		SDL_QueryTexture(enemy->texture, NULL, NULL, &enemy->w, &enemy->h);

		enemy->dx = -(2 + (rand() % 4));
                enemy->side = SIDE_ALIEN;
                enemy->health = 1;
		enemy->reload = FPS * (1 + (rand() % 3));
		enemySpawnTimer = 30 + (rand() %FPS);
std::cout<<"spawnEnemies!"<<std::endl;
}
}

static void drawFighters(void)
{
	
	std::cout<<"drawFighters!"<<std::endl;
	Laser *e;

	for(e = stage.fighterHead.next ; e != NULL ; e = e->next)
	{
	blit(e->texture, e->x, e->y);
	std::cout<<"for loop drawFighters!"<<std::endl;
	}
}

static void doPlayer(void)
{
std::cout<<"doPlayer!"<<std::endl;
    if(player != NULL)
    {
    player->dx = player->dy = 0;

    	if (player->reload > 0)
    	{
        player->reload--;
    	}

    	if (dot.keyboard[SDL_SCANCODE_UP])
    	{
        player->dy = -PLAYER_SPEED;
    	}

    	if (dot.keyboard[SDL_SCANCODE_DOWN])
    	{
        player->dy = PLAYER_SPEED;
    	}

    	if (dot.keyboard[SDL_SCANCODE_LEFT])
    	{
        player->dx = -PLAYER_SPEED;
    	}

    	if (dot.keyboard[SDL_SCANCODE_RIGHT])
    	{
        player->dx = PLAYER_SPEED;
    	}

    	if (dot.keyboard[SDL_SCANCODE_LCTRL] && player->reload <= 0)
    	{
        fireBullet();
	std::cout<<"Ctrl key pressed!"<<std::endl;
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

		if (bulletHitFighter(b) || b->x < -b->w || b->y < -b->h || b->x > SCREEN_WIDTH || b->y > SCREEN_HEIGHT)
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
std::cout<<"fireBullet"<<std::endl;
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
}

//static void drawPlayer(void)
//{
//    blit(player->texture, player->x, player->y);
//}

static void drawBullets(void)
{
    std::cout<<"drawBullets"<<std::endl;
    Laser *b;

    for (b = stage.bulletHead.next ; b != NULL ; b = b->next)
    {
        blit(b->texture, b->x, b->y);
    }
}

static int bulletHitFighter(Laser *b)
{
	Laser * e;

	for(e = stage.fighterHead.next ; e != NULL ; e = e->next)
	{
		if(e->side != b->side && collision(b->x, b->y, b->w, b->h, e->x, e->y, e->w, e->h))
		{
			b->health = 0;
			e->health = 0;
			
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

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);

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

while(stage.fighterHead.next)
{
	e = stage.fighterHead.next;
	stage.fighterHead.next = e->next;
	delete e;
}

while(stage.bulletHead.next)
{
	e = stage.bulletHead.next;
	stage.bulletHead.next = e->next;
	delete e;
}

stage.fighterTail = &stage.fighterHead;
stage.bulletTail = &stage.bulletHead;

initPlayer();

enemySpawnTimer = 0;

stageResetTimer = FPS * 2;
}

static void doEnemies(void)
{
	Laser *e;
	
	for(e = stage.fighterHead.next ; e != NULL ; e = e->next)
	{
	if(e != player && player != NULL && --e->reload <= 0)
	{
	fireAlienBullet(e);
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
}

static void clipPlayer(void)
{
	if(player != NULL)
	{
		if(player->x <0)
		{
		player->x = 0;
		}
		if(player->y < 0)
		{
		player->y = 0;
		}

		if(player->x > SCREEN_WIDTH / 2)
		{
			player->x = SCREEN_WIDTH / 2;
		}

		if(player->y > SCREEN_HEIGHT - player->h)
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