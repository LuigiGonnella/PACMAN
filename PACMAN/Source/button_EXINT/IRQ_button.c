#include "button.h"
#include "LPC17xx.h"
#include "GLCD/GLCD.h" 
#include "GLCD/AsciiLib.h"
#include "timer/timer.h"
#include "RIT/RIT.h"



void EINT0_IRQHandler (void)	  	/* INT0														 */
{
	static int first=0; //flag per togliere cancello
	
	first++;
	if(pause==1) { //se sono in pausa 
			enable_RIT();										/* enable RIT to count 50ms				 */
		if (first==1) {
			startGhost(); //tolgo cancello
			enable_timer(3);


		}
	}
	NVIC_DisableIRQ(EINT0_IRQn);		/* disable Button interrupts			 */
	LPC_PINCON->PINSEL4    &= ~(1 << 20);     /* GPIO pin selection */

	
	LPC_SC->EXTINT &= (1 << 0);     /* clear pending interrupt         */
}


void EINT1_IRQHandler (void)	  	/* KEY1														 */
{
	
	LPC_SC->EXTINT &= (1 << 1);     /* clear pending interrupt         */
}

void EINT2_IRQHandler (void)	  	/* KEY2														 */
{
	
  LPC_SC->EXTINT &= (1 << 2);     /* clear pending interrupt         */    
}


