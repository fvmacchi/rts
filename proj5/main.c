#include <LPC17XX.H>
#include "glcd.h"
#include <RTL.h>
#include <stdio.h>
#include "linked_list.h"
#include "ball.h"

int SCREEN_WIDTH = 320;
int SCREEN_HEIGHT = 240;

const int MAX_SIZE = 50;
unsigned short clear_bitmap[MAX_SIZE*MAX_SIZE];

OS_MUT listMut, drawMut;

linked_list_t ball_list;

volatile int createball = 0;

__task void newBall( void *pointer ) {
	ball_t ball;
	ball_init(&ball, 31, 0, 0, 1, 1);
	os_mut_wait(&listMut, 0xffff);
	list_add(&ball_list, &ball);
	os_mut_release(&listMut);
	while(1) {
		os_mut_wait(&drawMut,0xffff);
		GLCD_Bitmap(ball.x,ball.y,ball.size,ball.size,(unsigned char *)clear_bitmap);
		ball.x += ball.velx;
		ball.y += ball.vely;
		GLCD_Bitmap(ball.x,ball.y,ball.size,ball.size,ball.bitmap);
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
		if(createball) {
			os_tsk_create_ex(newBall, 10, NULL);
			createball = 0;
		}
		os_mut_wait(&listMut, 0xffff);
		{
			ball = list_reset(&ball_list);
			while(ball != NULL) {
				ball = list_next(&ball_list);
			}
		}
		os_mut_release(&listMut);
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
	for(i = 0; i < MAX_SIZE*MAX_SIZE; i++) {
		clear_bitmap[i] = White;
	}
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
