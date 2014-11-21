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

OS_MUT drawMut;

int numBalls = 0;

volatile int createball = 0;
volatile double lastInterupt;
volatile unsigned char ADC_Done = 0; 
volatile unsigned short int ADC_Value = 1000;
const unsigned char ledPosArray[8] = { 28, 29, 31, 2, 3, 4, 5, 6 };


// INT0 interrupt handler
void EINT3_IRQHandler( void ) {
	// Check whether the interrupt is called on the falling edge. GPIO Interrupt Status for Falling edge.
	if ( LPC_GPIOINT->IO2IntStatF && (0x01 << 10) ) {
		LPC_GPIOINT->IO2IntClr |= (1 << 10); // clear interrupt condition
		
		createball = 1;
	}
}

// Starting the conversion. Upon the call of this function, the ADC unit starts
// to read the connected port to its channel. The conversion takes 56 clock ticks.
// According the initialization, an intrupt will be called when the data becomes 
// ready.
void ADCConvert (void) {
	// Stop reading and converting the port channel AD0.2.
  LPC_ADC->ADCR &= ~( 7 << 24); 
	ADC_Done = 0;
	// Start reading and converting the analog input from P0.25, where Poti is connected
	//to the challen Ad0.2
  LPC_ADC->ADCR |=  ( 1 << 24) | (1 << 2);              /* start conversion              */
}

void ADC_IRQHandler( void ) {
	volatile unsigned int aDCStat;

	// Read ADC Status clears the interrupt
	aDCStat = LPC_ADC->ADSTAT;

	// Read the value and and witht a max value as 12-bit.
	ADC_Value = (LPC_ADC->ADGDR >> 4) & 0xFFF; 

	ADC_Done = 1;
}

// Turn on the LED inn a position within 0..7
void turnOnLED( unsigned char led ) {
	unsigned int mask = (1 << ledPosArray[led]);

	// The first two LEDs are connedted to the port 28, 29 and 30
	if ( led < 3 ) {
		// Fast Port Output Set register controls the state of output pins.
		// Writing 1s produces highs at the corresponding port pins. Writing 0s has no effect (Section 9.5)
		LPC_GPIO1->FIOSET |= mask;
	} else {
		LPC_GPIO2->FIOSET |= mask;
	}

}

// Turn off the LED in the position within 0..7
void turnOffLED( unsigned char led ) {
	unsigned int mask = (1 << ledPosArray[led]);

	// The first two LEDs are connedted to the port 28, 29 and 30
	if ( led < 3 ) {
		// Fast Port Output Clear register controls the state of output pins. 
		// Writing 1s produces lows at the corresponding port pins (Section 9.5)
		LPC_GPIO1->FIOCLR |= mask;
	} else {
		LPC_GPIO2->FIOCLR |= mask;
	}
}

__task void newBall( void *pointer ) {
	ball_t ball;
	int x0, y0;
	int x, y, width, height;
	unsigned int multiplier;
	volatile double lastTime = os_time_get();
	volatile double dt = 0;
	ball_init(&ball, 25, Blue, 0, 0, 1, 1);
	while(1) {
		os_mut_wait(&drawMut,0xffff);
		dt = os_time_get() - lastTime;
		if(dt < 2) {
			os_mut_release(&drawMut);
			os_tsk_pass();
			continue;
		}
		lastTime = os_time_get();
		bitmap_circle(bitmap, ball.size, ball.colour);
		x0 = ball.x;
		y0 = ball.y;
		multiplier = ADC_Value/1000;
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

__task void physics( void ) {
	ball_t *ball = NULL;
	OS_TID task;
	int i;
	while(1) {
		if(createball && os_time_get() - lastInterupt > 25) {
			lastInterupt = os_time_get();
			createball = 0;
			task = os_tsk_create_ex(newBall, 10, NULL);
			if(task) {
				numBalls++;
				for(i = 0; i < 8; i++) {
					if(numBalls & (1 << i)) {
						turnOnLED(i);
					}
					else {
						turnOffLED(i);
					}
				}
			}
		}
		createball = 0;
		os_tsk_pass();
	}
}

__task void init_task( void ) {
	int i;
	os_mut_init(&drawMut);
	lastInterupt = os_time_get();
	os_tsk_create(physics,10);
}

int main( void ) {
	SystemInit();
	SystemCoreClockUpdate();
	GLCD_Init();
	GLCD_Clear(White);

	
	// INTERUPT BUTTON
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
	
	// POTENTIOMETER
	// Enabled the Power controler in PCONP register. According the Table 46. the 12th bit is PCADC
	LPC_SC->PCONP |= (1 << 12);

	// Poti is connected to port P0.25. We have to put the port P0.25 into the AD0.2 moe for anlaoge to digital conterting.
	LPC_PINCON->PINSEL1 &= ~(0x3 << 18); // Remove all bits, Port P0.25 gets GPIO
	LPC_PINCON->PINSEL1 |=  (0x1 << 18); // Switch P0.25 to AD0.2

	// No pull-up no pull-down (function 10) on the AD0.2 pin.
	LPC_PINCON->PINMODE1 &= ~(0x3 << 18);
	LPC_PINCON->PINMODE1 |=  (0x1 << 18);

	// A/D Control Register (Section 29.5.1)
	LPC_ADC->ADCR = ( 1 <<  2)  |    // SEL=1        select channel 0~7 on AD0.2 
	                ( 4 <<  8)  |    // ADC clock is 25 MHz/5          
	                ( 0 << 16 ) |    // BURST = 0    no BURST, software controlled 
	                ( 0 << 24 ) |    // START = 0    A/D conversion stops */
	                ( 0 << 27 ) |    // EDGE = 0     CAP/MAT singal falling,trigger A/D conversion
	                ( 1 << 21);      // PDN = 1      normal operation, Enable ADC                

	// Enabling A/D Interrupt Enable Register for all channels (Section 29.5.3)
	LPC_ADC->ADINTEN = ( 1 <<  8);        

	// Registering the interrupt service for ADC
	NVIC_EnableIRQ( ADC_IRQn );       

	// LED's
	// LPC_SC is a general system-control register block, and PCONP referes
	// to Power CONtrol for Peripherals.
	//  - Power/clock control bit for IOCON, GPIO, and GPIO interrupts (Section 4.8.9)
	//    This can also be enabled from `system_LPC17xx.c'
	LPC_SC->PCONP     |= (1 << 15);            

	// The ports connected to p1.28, p1.29, and p1.31 are in mode 00 which
	// is functioning as GPIO (Section 8.5.5)
	LPC_PINCON->PINSEL3 &= ~(0xCF00);

	// The port connected to p2.2, p2.3, p2.4, p2.5, and p2.6 are in mode 00
	// which is functioning as GPIO (Section 8.5.5)
	LPC_PINCON->PINSEL4 &= (0xC00F);

	// LPC_GPIOx is the general control register for port x (Section 9.5)
	// FIODIR is Fast GPIO Port Direction control register. This register 
	// individually controls the direction of each port pin (Section 9.5)
	//
	// Set the LEDs connected to p1.28, p1.29, and p1.31 as output
	LPC_GPIO1->FIODIR |= 0xB0000000;           

	// Set the LEDs connected to p2.2, p2.3, p2.4, p2.5, and p2.6 as output port
	LPC_GPIO2->FIODIR |= 0x0000007C;           

	os_sys_init(init_task);
}
