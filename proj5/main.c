#include <LPC17XX.H>
#include "glcd.h"
#include <RTL.h>
#include <stdio.h>

volatile int createball = 0;

const int size = 51;
unsigned int position = 0;
unsigned short ball_bitmap[size*size];

//SIZE MUST BE AN ODD NUMBER
void create_circle_bitmap(unsigned short *bitmap, int size) {
	int i, k;
	int radius = size/2;
	for(i = 0; i < size*size; i++) {
		bitmap[i] = Red;
	}
	for(i = -radius; i <= radius; i++) {
		for(k = -radius; k <= radius; k++) {
			bitmap[(i+radius)*size+k+radius] = (i*i + k*k < radius*radius) ? Blue : White;
		}
	}
}

__task void newBall( void *pointer ) {
	GLCD_Bitmap(position,position,size,size,(unsigned char*)ball_bitmap);
	position += size;
	os_tsk_delete_self();
}

// INT0 interrupt handler
void EINT3_IRQHandler( void ) {
	// Check whether the interrupt is called on the falling edge. GPIO Interrupt Status for Falling edge.
	if ( LPC_GPIOINT->IO2IntStatF && (0x01 << 10) ) {
		LPC_GPIOINT->IO2IntClr |= (1 << 10); // clear interrupt condition
		
		createball = 1;
	}
}

__task void init_task( void ) {
	create_circle_bitmap(ball_bitmap, size);
	while(1) {
		if(createball) {
			os_tsk_create_ex(newBall, 10, NULL);
			createball = 0;
		}
	}
}

int main( void ) {
	SystemInit();
	SystemCoreClockUpdate();
	GLCD_Init();
	GLCD_Clear(White);
	
	// P2.10 is related to the INT0 or the push button.
	// P2.10 is selected for the GPIO 
	LPC_PINCON->PINSEL4 &= ~(3<<20); 

	// P2.10 is an input port
	LPC_GPIO2->FIODIR   &= ~(1<<10); 

	// P2.10 reads the falling edges to generate the IRQ
	// - falling edge of P2.10
	LPC_GPIOINT->IO2IntEnF |= (1 << 10);

	// IRQ is enabled in NVIC. The name is reserved and defined in `startup_LPC17xx.s'.
	// The name is used to implemet the interrupt handler above,
	NVIC_EnableIRQ( EINT3_IRQn );
	
	os_sys_init(init_task);
}
