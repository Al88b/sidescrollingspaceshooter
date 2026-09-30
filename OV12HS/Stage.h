#ifndef STAGE_H_
#define STAGE_H_

class Stage
{
public:

    	Laser fighterHead, *fighterTail;
    	Laser bulletHead, *bulletTail;
	Explosion explosionHead, *explosionTail;
	Debris debrisHead, *debrisTail;
	int score;
};

#endif