#ifndef LASER_H_
#define LASER_H_

class Laser
{
public:

float x;
float y;
int w;
int h;
float dx;
float dy;
int health;
int reload;
int side;
SDL_Texture* texture;
Laser *next;
int lives = 5;
};

#endif