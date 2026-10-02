#ifndef TEXTURE_H
#define TEXTURE_H

class Texture
{
public:
char name[MAX_NAME_LENGTH];
SDL_Texture *texture;
Texture *next;
};

#endif