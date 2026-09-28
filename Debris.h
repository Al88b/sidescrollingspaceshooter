#ifndef DEBRIS_H_
#define  DEBRIS_H_

class Debris
{
	public:

	float x;
	float y;
	float dx;
	float dy;
	SDL_Rect rect;
	SDL_Texture *texture;
	int life;
	Debris *next;
};

#endif