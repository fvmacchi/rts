#include <LPC17XX.H>
#include <stdio.h>
#include "GLCD.h"

#ifndef BALL_H
#define BALL_H

typedef struct ball{
	int x, y;
	int velx, vely;
	int size;
	unsigned char *bitmap;
}ball_t;

void ball_init(ball_t *ball, int size, int x, int y, int velx, int vely);

void ball_draw(ball_t *ball);

void ball_update(ball_t *ball);

#endif