//Using SDL, SDL_image, standard IO, vectors, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <iostream>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LTexture.h"
#include "LTexture.cpp"
#include "FunctionDeclarations.h"
#include "FunctionDefinitions.cpp"
#include "Delegate.h"
#include "Dot.cpp"
#include "Laser.h"
#include "Explosion.h"
#include "Debris.h"
#include "Stage.h"

int main(int argc, char** args)
{

//Start up SDL and create window
if(!init())
{
printf("Failed to initialize!\n");
}

//Start up SDL and create window
if(!init())
{
printf("Failed to initialize!\n");
}

//Load media
if(!loadMedia())
{
printf("Failed to load media!\n");
}

//Main loop flag
bool quit = false;

    long then;
    float remainder;

    atexit(cleanup);

    initStage();

    then = SDL_GetTicks();

    remainder = 0;

		//Event handler
		SDL_Event e;


		//The background scrolling offset
		int scrollingOffset = 0;

		laser.texture = loadTexture("playerLaser.png");

		//While application is running
		while(!quit)
		{
			//Handle events on queue
			while(SDL_PollEvent(&e) != 0)
			{
				//User requests quit
				if(e.type == SDL_QUIT)
				{
					quit = true;
				}

				//Handle input for the dot
				dot.handleEvent(e);

			}


			//Move the dot
			//dot.move();

			//Scroll background
			--scrollingOffset;
			if(scrollingOffset < -gBGTexture.getWidth())
			{
				scrollingOffset = 0;
			}

			//Clear screen
			SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
			SDL_RenderClear(gRenderer);

			//Render background
			gBGTexture.render(scrollingOffset, 0);
			gBGTexture.render(scrollingOffset + gBGTexture.getWidth(), 0);

			//Render objects
			//dot.render();

			dot.x += dot.dx;
			dot.y += dot.dy;

			
        prepareScene();

        dot.delegate.logic();

        dot.delegate.draw();	

        presentScene();

        capFrameRate(&then, &remainder);

			//Update screen
			SDL_RenderPresent(gRenderer);

		}
	


//Free resources and close SDL
close();

return 0;

}

