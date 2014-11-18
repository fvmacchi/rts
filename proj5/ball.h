#include <LPC17XX.H>

#ifndef BALL_H
#define BALL_H

typedef struct ball{
	int x, y;
	int velx, vely;
	unsigned short *bitmap;
}ball_t;

#endif