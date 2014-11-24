#include <LPC17XX.H>
#include <stdio.h>
#include "ball.h"

void ball_init(ball_t *ball, int size, unsigned short colour, int x, int y, int velx, int vely) {
	ball->x = x;
	ball->y = y;
	ball->velx = velx;
	ball->vely = vely;
	ball->size = size;
	ball->colour = colour;
	ball->collision = 0;
}
