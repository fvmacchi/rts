#include <LPC17XX.H>
#include <stdio.h>
#include "ball.h"

//SIZE MUST BE AN ODD NUMBER
void create_circle_bitmap(unsigned short *bitmap, int size) {
	int i, k;
	int radius = size/2;
	for(i = -radius; i <= radius; i++) {
		for(k = -radius; k <= radius; k++) {
			bitmap[(i+radius)*size+k+radius] = (i*i + k*k < radius*radius) ? Blue : White;
		}
	}
}

void ball_init(ball_t *ball, int size, int x, int y, int velx, int vely) {
	do {
		ball->bitmap = (unsigned char *)malloc(sizeof(unsigned short)*size*size);
	} while(ball->bitmap == NULL);
	create_circle_bitmap((unsigned short *)ball->bitmap, size);
	ball->x = x;
	ball->y = y;
	ball->velx = velx;
	ball->vely = vely;
	ball->size = size;
}