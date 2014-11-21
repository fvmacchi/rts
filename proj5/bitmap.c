#include "bitmap.h"

void bitmap_clear(unsigned short *bitmap, int size) {
  int i;
  for(i = 0; i < size*size; i++) {
    bitmap[i] = White;
  }
}

//SIZE MUST BE AN ODD NUMBER
void bitmap_circle(unsigned short *bitmap, int size, unsigned short colour) {
  int i, k;
  int radius = size/2;
  for(i = -radius; i <= radius; i++) {
    for(k = -radius; k <= radius; k++) {
      bitmap[(i+radius)*size+k+radius] = (i*i + k*k < radius*radius) ? Blue : White;
    }
  }
}
