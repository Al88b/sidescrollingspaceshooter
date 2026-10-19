#ifndef DELEGATE_H
#define DELEGATE_H

class Delegate
{
public:
void (*logic)(void);
void (*draw)(void);
void (*drawScores)(void);
void (*logicTitle)(void);
void (*logicScores)(void);
void (*drawTitle)(void);
};

#endif