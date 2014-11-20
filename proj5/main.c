#include <LPC17XX.H>
#include "glcd.h"
#include <RTL.h>
#include <stdio.h>
#include "bitmap.h"
#include "linked_list.h"
#include "ball.h"

int SCREEN_WIDTH = 320;
int SCREEN_HEIGHT = 240;

const int MAX_SIZE = 55;
unsigned short bitmap[MAX_SIZE*MAX_SIZE];

OS_MUT listMut, drawMut;

linked_list_t ball_list;

volatile int createball = 0;
volatile double lastInterupt;

__task void newBall( void *pointer ) {
	ball_t ball;
	int x0, y0;
	int x, y, width, height;
	volatile double lastTime = os_time_get();
	volatile double dt = 0;
	ball_init(&ball, 31, Blue, 0, 0, 1, 1);
	os_mut_wait(&listMut, 0xffff);
	list_add(&ball_list, &ball);
	os_mut_release(&listMut);
	while(1) {
		os_mut_wait(&drawMut,0xffff);
		dt = os_time_get() - lastTime;
		if(dt < 1) {
			os_mut_release(&drawMut);
			os_tsk_pass();
			continue;
		}
		lastTime = os_time_get();
		bitmap_circle(bitmap, ball.size, ball.colour);
		x0 = ball.x;
		y0 = ball.y;
		ball.x += ball.velx*dt;
		ball.y += ball.vely*dt;
		if(ball.x < 0 || ball.x + ball.size > SCREEN_WIDTH) {
			ball.velx = -ball.velx;
			ball.x += ball.velx*dt;
		}
		if(ball.y < 0 || ball.y + ball.size > SCREEN_HEIGHT) {
			ball.vely = -ball.vely;
			ball.y += ball.vely*dt;
		}
		GLCD_Bitmap(ball.x,ball.y,ball.size,ball.size,(unsigned char *)bitmap);
		bitmap_clear(bitmap, ball.size);
		
		// Clear previous ball image
		if(ball.x > x0 + ball.size || ball.x + ball.size < x0 || ball.y > y0 + ball.size || ball.y + ball.size < y0) {
			GLCD_Bitmap(x0,y0,ball.size,ball.size, (unsigned char *)bitmap);
		}
		else {
			y = y0;
			height = ball.size;
			if(x0 < ball.x) {
				x = x0;
				width = ball.x - x0 + 1;
			}
			else {
				x = ball.x + ball.size;
				width = x0 - ball.x + 1;
			}
			if(x + width > SCREEN_WIDTH) {
				width -= x + width - SCREEN_WIDTH;
			}
			if(width > 0) {
				GLCD_Bitmap(x,y,width,height,(unsigned char *)bitmap);
			}
			
			y = y0;
			height = ball.size;
			if(x0 < ball.x) {
				x = x0;
				width = ball.x - x0;
			}
			else {
				x = ball.x + ball.size;
				width = x0 - ball.x;
			}
			if(y0 < ball.y) {
				y = y0;
				height = ball.y - y0;
			}
			else {
				y = ball.y + ball.size;
				height = y0 - ball.y;
			}
			if(x0 < ball.x) {
				x = ball.x;
				width = ball.size - ball.x + x0;
			}
			else {
				x = x0;
				width = ball.size - x0 + ball.x;
			}
			GLCD_Bitmap(x,y,width,height,(unsigned char *)bitmap);
		}
		os_mut_release(&drawMut);
		os_tsk_pass();
	}
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

__task void physics( void ) {
	ball_t *ball = NULL;
	while(1) {
		if(createball && os_time_get() - lastInterupt > 25) {
			lastInterupt = os_time_get();
			createball = 0;
			os_tsk_create_ex(newBall, 10, NULL);
		}
		createball = 0;
		/*
		os_mut_wait(&listMut, 0xffff);
		{
			ball = list_reset(&ball_list);
			while(ball != NULL) {
				ball = list_next(&ball_list);
			}
		}
		os_mut_release(&listMut);
		*/
		os_tsk_pass();
	}
}

__task void init_task( void ) {
	int i;
	os_mut_init(&listMut);
	os_mut_init(&drawMut);
	os_mut_wait(&listMut, 0xffff);
	list_init(&ball_list);
	os_mut_release(&listMut);
	lastInterupt = os_time_get();
	os_tsk_create(physics,10);
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
