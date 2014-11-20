#include <LPC17XX.H>
#include <stdio.h>
#include "GLCD.h"

unsigned short *get_bitmap(int size);

void bitmap_clear(unsigned short *bitmap, int size);

void bitmap_circle(unsigned short *bitmap, int size, unsigned short colour);
