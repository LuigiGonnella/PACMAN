/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "LPC17xx.h"
#include "timer.h"
#include "GLCD/GLCD.h" 
#include "GLCD/AsciiLib.h"
#include "RIT/RIT.h"

/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

extern unsigned char led_value;					/* defined in funct_led								*/
int seconds=60, offset=0, moveg=0, speed=0, flagm=1; //secondi rimanenti, offset per entrare ogni 4 volte nel timer che scandisce i secondi,
//counter per muovere i fantasmi alle varie velocità, flag varie velocità, flag per aspettare i 3s di respawn

unsigned char ledval = 0xA5;
uint32_t val_TC; 
int victory=2; //flag vittoria (superfluo)
int pillarsMrem=6; //pillar medi da generare

uint16_t SinTable[45] =                                       /* ÕýÏÒ±í                       */
{
    410, 467, 523, 576, 627, 673, 714, 749, 778,
    799, 813, 819, 817, 807, 789, 764, 732, 694, 
    650, 602, 550, 495, 438, 381, 324, 270, 217,
    169, 125, 87 , 55 , 30 , 12 , 2  , 0  , 6  ,   
    20 , 41 , 70 , 105, 146, 193, 243, 297, 353
};

void TIMER0_IRQHandler (void)
{
	if(LPC_TIM0->IR & 1) // MR0
	{ 
		static int sineticks=0;
		/* DAC management */	
		static int currentValue; 
		currentValue = SinTable[sineticks];
		currentValue -= 410;
		currentValue /= 1;
		currentValue += 410;
		LPC_DAC->DACR = currentValue <<6;
		sineticks++;
		if(sineticks==45) sineticks=0;
		LPC_TIM0->IR = 1;			//clear interrupt flag
	}
	else if(LPC_TIM0->IR & 2){ // MR1
		// your code	
		LPC_TIM0->IR = 2;			// clear interrupt flag 
	}
	else if(LPC_TIM0->IR & 4){ // MR2
		// your code	
		LPC_TIM0->IR = 4;			// clear interrupt flag 
	}
	else if(LPC_TIM0->IR & 8){ // MR3
		// your code	
		LPC_TIM0->IR = 8;			// clear interrupt flag 
	}
  return;
}

/******************************************************************************
** Function name:		Timer1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER1_IRQHandler (void)
{
	int first, second;
	
	
	
	if(LPC_TIM1->IR & 1) // MR0
	{ 
		if(offset==4) { //ogni 4 volte entro decremento secondi di 1 (così ho margine per muovere i fantasmi più veloce)
			first=(seconds/10);
			second=(seconds%10);
			
			offset=0;
			
						
			PutChar(48,20,first+48,Red,Black);
			PutChar(57,20,second+48,Red,Black);
			PutChar(66,20,115,Red,Black);
			

			seconds--;
			if(seconds==40) { //modalità di velocità 1
				speed=1;
			}
			else if(seconds==20) { //modalità di velocità 2
				speed=2;
			}
			if(seconds==secondpow) { //dopo 10s finishe frightned mode
				power=0;
				
			}
			if(seconds==secondspawn) { //dopo 3s respawna fantasma
				i_ghost=10;
				j_ghost=10;
				disegnaFantasma(i_ghost,j_ghost,Blue);
				secondspawn=-1;
				flagm=1;
				
			}
			if (seconds==0) { //allo scadere del tempo perdo
				victory=0;
				disable_timer(1);
				PutChar(48,20,48,Red,Black);
				PutChar(57,20,48,Red,Black);
				PutChar(66,20,115,Red,Black);
				//GAMEOVER
				
				disable_RIT();			// stoppo il gioco
				disable_timer(1);
				LCD_Clear(Black);
				GUI_Text(6*10+10, 12*10+40, (uint8_t *) " Game Over! ", Red, Black);
				

				
				
				
				
				
				
			}	
				
			}
		if(speed==0 && flagm>=1) { //modalità velocità iniziale
			if(moveg>=4) { //muovo fantasma ogni 1s
				moveg=0;
				moveGhost();
			}
		}
		else if(speed==1 && flagm>=1) { //modalità velocità 1
			if(moveg>=2) { //muovo fantasma ogni 0.5s
				moveg=0;
				moveGhost();
			}
		}
		else if(speed==2 && flagm>=1) { //modalità velocità 2
			if(moveg>=1) { //muovo fantasma ogni 0.125s
				moveg=0;
				moveGhost();
			}
		}
		offset++;
		moveg++;
			
		
		
			
		
		
		
		LPC_TIM1->IR = 1;			//clear interrupt flag
	}

	else if(LPC_TIM1->IR & 2){ // MR1
		//LED_Out(ledval);
		LPC_TIM1->IR = 2;			// clear interrupt flag 
	}
	else if(LPC_TIM1->IR & 4){ // MR2
		// your code	
		LPC_TIM1->IR = 4;			// clear interrupt flag 
	}
	else if(LPC_TIM1->IR & 8){ // MR3
		// your code	
		LPC_TIM1->IR = 8;			// clear interrupt flag 
	} 

	return;
}

/******************************************************************************
** Function name:		Timer2_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
// gestisce il debounce di INT0
void TIMER2_IRQHandler (void)
{
	if(LPC_TIM2->IR & 1) // MR0 -> lavoro con MR0 per il debounce
	{ 
		disable_timer(0); // duration della nota
		ticks = 1;
		
		LPC_TIM2->IR = 1;			//clear interrupt flag
	}
	
	else if(LPC_TIM2->IR & 2){ // MR1 per pillarsm casuali
		// your code	
		int i_rand,j_rand;
		if(pillarsMrem!=0) { 
			i_rand = (LPC_TIM0->TC)%N1; //lettura di TC ; 
			j_rand = (LPC_TIM1->TC)%N1; //lettura di TC ;
			for(; schermo[i_rand][j_rand] != 10; i_rand = rand() % N1, j_rand = rand() % N1); //quando trova un pilar nella casella c'è 10, appena esce trova la casella con il pillar
			disegnaPillarM(i_rand, j_rand);
			pillarsMrem--;
		}
		else{
			disable_timer(2);
		}
		
		LPC_TIM2->IR = 2;			// clear interrupt flag 
	}
	return;
}


/******************************************************************************
** Function name:		Timer2_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER3_IRQHandler (void)
{
  if(LPC_TIM3->IR & 1) // MR0
	{ 
		
		LPC_TIM3->IR = 1;			//clear interrupt flag
	}
	else if(LPC_TIM3->IR & 2){ // MR1
		//LED_On(1);
		LPC_TIM3->IR = 2;			// clear interrupt flag 
	}
	
  //LPC_TIM2->IR = 1;			/* clear interrupt flag */
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/

/*
void TIMER0_IRQHandler (void)
{
	if(LPC_TIM0->IR & 1) // MR0
	{ 
		// your code
		LPC_TIM0->IR = 1;			//clear interrupt flag
	}
	else if(LPC_TIM0->IR & 2){ // MR1
		// your code	
		LPC_TIM0->IR = 2;			// clear interrupt flag 
	}
	else if(LPC_TIM0->IR & 4){ // MR2
		// your code	
		LPC_TIM0->IR = 4;			// clear interrupt flag 
	}
	else if(LPC_TIM0->IR & 8){ // MR3
		// your code	
		LPC_TIM0->IR = 8;			// clear interrupt flag 
	}
  return;
}*/