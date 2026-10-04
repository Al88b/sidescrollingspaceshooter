#ifndef DELEGATE_H
#define DELEGATE_H

class Delegate
{
public:
void (*logic)(void);
void (*draw)(void);
void (*logicHighScore)(void);
void (*drawHighScore)(void);
};

#endif