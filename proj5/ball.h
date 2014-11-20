#include <LPC17XX.H>
#include <stdio.h>
#include "GLCD.h"

#ifndef BALL_H
#define BALL_H

typedef struct ball{
	int x, y;
	int velx, vely;
	int size;
	unsigned short colour;
}ball_t;

void ball_init(ball_t *ball, int size, unsigned short colour, int x, int y, int velx, int vely);

#endif
