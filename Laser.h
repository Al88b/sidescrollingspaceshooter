#ifndef LASER_H
#define LASER_H

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
};

#endif