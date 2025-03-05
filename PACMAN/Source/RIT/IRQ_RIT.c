/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "LPC17xx.h"
#include "RIT.h"
#include "GLCD/GLCD.h" 
#include "timer/timer.h"
#include "button_EXINT/button.h"


/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

volatile flag_start=0; //flag per primo avvio -> avvio counter secondi

// MUSICA
#define UPTICKS 1
static int currentNote = 0;
int ticks = 1;
NOTE melody[MELODY_SIZE] = 
{
		// 1
	{NOTE_D3, time_16th},
	{NOTE_D3, time_16th},
	{NOTE_D4, time_8th},
	{NOTE_A3, time_8th},
	{REST, time_16th},
	{NOTE_GS3, time_16th},
	{REST, time_16th},
	{NOTE_G3, time_8th},
	{NOTE_F3, time_16th * 2},
	{NOTE_D3, time_16th},
	{NOTE_F3, time_16th},
	{NOTE_G3, time_16th},

		// 2
	{NOTE_C3, time_16th},
	{NOTE_C3, time_16th},
	{NOTE_D4, time_8th},
	{NOTE_A3, time_8th},
	{REST, time_16th},
	{NOTE_GS3, time_16th},
	{REST, time_16th},
	{NOTE_G3, time_8th},
	{NOTE_F3, time_16th * 2},
	{NOTE_D3, time_16th},
	{NOTE_F3, time_16th},
	{NOTE_G3, time_16th},

		// 3
	{NOTE_C3B, time_16th},
	{NOTE_C3B, time_16th},
	{NOTE_D4, time_8th},
	{NOTE_A3, time_8th},
	{REST, time_16th},
	{NOTE_GS3, time_16th},
	{REST, time_16th},
	{NOTE_G3, time_8th},
	{NOTE_F3, time_16th * 2},
	{NOTE_D3, time_16th},
	{NOTE_F3, time_16th},
	{NOTE_G3, time_16th},

		// 4
	{NOTE_GS2, time_16th},
	{NOTE_GS2, time_16th},
	{NOTE_D4, time_8th},
	{NOTE_A3, time_8th},
	{REST, time_16th},
	{NOTE_GS3, time_16th},
	{REST, time_16th},
	{NOTE_G3, time_8th},
	{NOTE_F3, time_16th * 2},
	{NOTE_D3, time_16th},
	{NOTE_F3, time_16th},
	{NOTE_G3, time_16th},

		// 5 (remaining part of the song)
};

void RIT_IRQHandler (void)
{					
	static int position=-1; //1->UP, 2->RIGHT, 3->LEFT, 4->DOWN
	static int scoretmp,debounce=0;
	
	reset_RIT();
	if(victory==0) return; //se ho perso esco subito
	
	CAN_TxMsg.data[0] = seconds;  // 8 bits per tempo rimanente
	CAN_TxMsg.data[1] = current_lives_remaining; // 8 bits for vite rimanennti
	CAN_TxMsg.data[2] = (score >> 8);    // Upper 8 bits del punteggio
	CAN_TxMsg.data[3] = (score & 0xFF);  // Lower 8 bits del punteggio

	CAN_wrMsg(1, &CAN_TxMsg);    // CAN message in CAN1

	if (i_ghost==i_pac && j_ghost==j_pac && power==1 && flagm!=-1) { //se pacman sta attaccando il fantasma e l'ha raggiunto
		score+=100; //aumento score
		secondspawn=seconds-3; //aspetto 3s per respawnare fantasma
		flagm=-1; //metto flag per non far spawnare fantasma
	}
	
	if (i_ghost==i_pac && j_ghost==j_pac && power==0) { //se fantasma sta attaccando pacman e l'ha raggiunto
		CancellaCella(i_pac,j_pac);
		i_pac=10;
		j_pac=13;
		disegnaPacmanRight(i_pac,j_pac); //disegno pacman in cella iniziale
		dead=1; //flag morte
		cancellaPacmanLeftGrande(i_life,j_life);
		i_life-=3; //coordinata vita disegnata
							current_lives_remaining--;
		

		
		if(i_life<0) { //non ho più vite
			victory=0;
			disable_timer(1);
			disable_RIT();			// stoppo il gioco
				LCD_Clear(Black);
				GUI_Text(6*10+10, 12*10+40, (uint8_t *) " Game Over! ", Red, Black);
		}

		
		
	} 
	
	if(dead==1) { //se entro subito dopo essere morto metto position ad un valore non valido così pacman non parte da solo
		position=-1;
	}
	
	if((LPC_GPIO2->FIOPIN & (1<<10)) == 0){   //int0 con interrupt attivato
		if(debounce==0) {
			debounce=1;
			//reset_RIT();
					if(pause == 0){ // se non sono in pausa
						pause = 1; //metto in pausa
						disable_timer(1);
						disable_timer(2);
						//disable_RIT();
						//reset_RIT();
						GUI_Text(9*10+10, 12*10+34, (uint8_t *) "PAUSE", Yellow, Blue);
					}
					else{ //se sono in pausa
						pause = 0; //tolgo pausa
						enable_timer(1);
						enable_timer(2);
						GUI_Text(9*10+10, 12*10+32, (uint8_t *) "     ", Blue, Blue);
						cancellaPac(10, 11);
						cancellaPac(10, 12);

						cancellaPac(11, 11);
						cancellaPac(11, 12);
						cancellaPac(12, 11);
						cancellaPac(12, 12);
						
						
						
						cancellaPac(9, 11);
						cancellaPac(9, 12);
						
						disegna(9,11);
						disegna(9,12);
						disegna(10,12);
						disegna(11,12);
						disegna(12,12);
						disegna(12,11);
						
						
						
						
						disegna(12,11);
						
					}
			 }
			//LPC_RIT->RICTRL |= 0x1;
			//reset_RIT();
		}
	 	
	else {	/* button released */
			debounce=0;
			
			NVIC_EnableIRQ(EINT0_IRQn);							 /* riattivo interrupts */
			LPC_PINCON->PINSEL4    |= (1 << 20);     /* External interrupt 0 pin selection */
			
			
			//LPC_RIT->RICTRL |= 0x1;
		}
	

	
	
	
	
	
	if (countPillar==0) { //ho mangiato tutti i pillars
		victory = 1;								// segno vittoria
		disable_timer(1);						// stoppo timer1
		disable_RIT();
			LCD_Clear(Black);
			GUI_Text(7*10+10, 13*10+40, (uint8_t *) " Victory! ", Green, Black);
		
		
	}

		
	scoretmp=score; //memorizzo score precedente per controllare il superamento del migliaio
	
	
		
		
	
	if(pause==0) { //se non sono in pausa
		if((LPC_GPIO1->FIOPIN & (1<<29)) == 0 && position!=1){	//bottone UP del joystick premuto
		/* Joytick UP pressed */
		position=1;
		if (flag_start==0) { //la prima volta avvio secondi
			flag_start++;
			enable_timer(1);

		}
		
		if(position==1) { //UP
			if(schermo[i_pac][j_pac-1]!=-1) { //non è un muro
				score+=schermo[i_pac][j_pac-1];  //10 se pillarS, 50 se pillarM, 0 se cella vuota
				
				if(schermo[i_pac][j_pac-1]>0) { //se ho mangiato qualcosa
					countPillar--;
					
				}
				cancellaPac(i_pac, j_pac); //cancello pacman nella vecchia casella
				
				disegnaPacmanUp(i_pac, j_pac-1);
				j_pac-=1; //vado sopra
				
				PutChar(160,20,(score/100)%10+48,Red,Black);
				PutChar(168,20,(score/10)%10+48,Red,Black);
				PutChar(152,20,(score/1000)%10+48,Red,Black);
				PutChar(176,20,score%10+48,Red,Black);
				
				
				if(score!=0 && scoretmp%1000>score%1000 && score!=scoretmp) { //se ho superato migliaio acquisisco vita
					i_life+=3;
					disegnaPacmanLeftGrande(i_life,j_life);
					current_lives_remaining++;
				}
				
				
			}
			
			else { //se è un muro disegno pacman up
				cancellaPac(i_pac, j_pac);
				disegnaPacmanUp(i_pac, j_pac);
				
			}
			
		}
		return;
		
		
		
	}
	else{ //pulsante UP rilasciato
		

	}
	
	//GLI ALTRI PULSANTI SONO ANALOGHI
	
	if ((LPC_GPIO1->FIOPIN & (1<<28)) == 0 && position!=2) { //pulsante RIGHT
		position=2;
		if (flag_start==0) {
			flag_start++;
			enable_timer(1);

		}
		
		if (i_pac==21) {
			cancellaPac(i_pac, j_pac);
			i_pac=-1;
			
		}
		
		if(position==2) { //RIGHT
			
			if(schermo[i_pac+1][j_pac]!=-1) { //non è un muro
				score+=schermo[i_pac+1][j_pac];
			if(schermo[i_pac+1][j_pac]>0) {
					countPillar--;
					
				}
				if (i_pac!=-1) {
					cancellaPac(i_pac, j_pac); //cancello pacman nella vecchia casella

				}
				
				
					i_pac+=1;

					disegnaPacmanRight(i_pac, j_pac);

				
				
				
				PutChar(160,20,(score/100)%10+48,Red,Black);
				PutChar(168,20,(score/10)%10+48,Red,Black);
				PutChar(152,20,(score/1000)%10+48,Red,Black);
				PutChar(176,20,score%10+48,Red,Black);
				
				if(score!=0 && scoretmp%1000>score%1000 && score!=scoretmp) {
					i_life+=3;
					disegnaPacmanLeftGrande(i_life,j_life);
										current_lives_remaining++;

				}
				
			}
			
			else { //se è un muro disegno pacman right
				cancellaPac(i_pac, j_pac);
				disegnaPacmanRight(i_pac, j_pac);
				
			}
			
		}
		return;

	}
	else {
	}
	
	if ((LPC_GPIO1->FIOPIN & (1<<27)) == 0 && position!=3) { //pulsante LEFT
		position=3;
		if (flag_start==0) {
			flag_start++;
			enable_timer(1);

		}
		
		if (i_pac==0) {
			cancellaPac(i_pac, j_pac);
			i_pac=22;
			
		}
		
		if(position==3) { //LEFT
			if(schermo[i_pac-1][j_pac]!=-1) { //non è un muro
				score+=schermo[i_pac-1][j_pac];
			if(schermo[i_pac-1][j_pac]>0) {
					countPillar--;
					
				}
				if (i_pac!=22) {
					cancellaPac(i_pac, j_pac); //cancello pacman nella vecchia casella

				}
				
				
				
					i_pac-=1;

					disegnaPacmanLeft(i_pac, j_pac);

				
				
				PutChar(160,20,(score/100)%10+48,Red,Black);
				PutChar(168,20,(score/10)%10+48,Red,Black);
				PutChar(152,20,(score/1000)%10+48,Red,Black);
				PutChar(176,20,score%10+48,Red,Black);
				
				
				if(score!=0 && scoretmp%1000>score%1000 && score!=scoretmp) {
					i_life+=3;
					disegnaPacmanLeftGrande(i_life,j_life);
										current_lives_remaining++;

				}
			}
			
			else { //se è un muro disegno pacman left
				cancellaPac(i_pac, j_pac);
				disegnaPacmanLeft(i_pac, j_pac);
				
			}
			
		}
		return;
	

	}
	else {

	}
	
	if ((LPC_GPIO1->FIOPIN & (1<<26)) == 0 && position!=4) { //pulsante DOWN
		position=4;
		if (flag_start==0) {
			flag_start++;
			enable_timer(1);

		}
		
		if(position==4) { //DOWN
			if(schermo[i_pac][j_pac+1]!=-1) { //non è un muro
				score+=schermo[i_pac][j_pac+1];
			if(schermo[i_pac][j_pac+1]>0) {
					countPillar--;
					
				}
				cancellaPac(i_pac, j_pac); //cancello pacman nella vecchia casella
				
				disegnaPacmanDown(i_pac, j_pac+1);
				j_pac+=1;
				
				PutChar(160,20,(score/100)%10+48,Red,Black);
				PutChar(168,20,(score/10)%10+48,Red,Black);
				PutChar(152,20,(score/1000)%10+48,Red,Black);
				PutChar(176,20,score%10+48,Red,Black);
				
				if(score!=0 && scoretmp%1000>score%1000 && score!=scoretmp) {
					i_life+=3;
					disegnaPacmanLeftGrande(i_life,j_life);
										current_lives_remaining++;

				}
				
			}
			
			else { //se è un muro disegno pacman down
				cancellaPac(i_pac, j_pac);
				disegnaPacmanDown(i_pac, j_pac);
				
			}
			
		}
		return;


	}
	else {
		
	}
	
	if(pause==0 && dead==0) {
		
		// MUSICA (parte solo dopo la generazione degli energy pillars
		if(pillarsMrem==0){
			if(ticks == 1)
			{
				ticks = 0;
				playNote(&melody[currentNote++]);
			}
		
			if (currentNote >= sizeof(melody) / sizeof(melody[0])) {
						currentNote = 0;  // Reset dell'indice per ricominciare la melodia
				}
		}
		if(position==1) { //UP
			if(schermo[i_pac][j_pac-1]!=-1) { //non è un muro
				score+=schermo[i_pac][j_pac-1];
			if(schermo[i_pac][j_pac-1]>0) {
					countPillar--;
					
				}
				cancellaPac(i_pac, j_pac); //cancello pacman nella vecchia casella
				
				disegnaPacmanUp(i_pac, j_pac-1);
				j_pac-=1;
				
				PutChar(160,20,(score/100)%10+48,Red,Black);
				PutChar(168,20,(score/10)%10+48,Red,Black);
				PutChar(152,20,(score/1000)%10+48,Red,Black);
				PutChar(176,20,score%10+48,Red,Black);
				
				
				if(score!=0 && scoretmp%1000>score%1000 && score!=scoretmp) {
					i_life+=3;
					disegnaPacmanLeftGrande(i_life,j_life);
										current_lives_remaining++;

				}
				
				
			}
			
			else { //se è un muro disegno pacman up
				cancellaPac(i_pac, j_pac);
				disegnaPacmanUp(i_pac, j_pac);
				
			}
			
		}
		
		
		if(position==2) { //RIGHT
			if (i_pac==21) {
			cancellaPac(i_pac, j_pac);
			i_pac=-1;
			
		}
			if(schermo[i_pac+1][j_pac]!=-1) { //non è un muro
				score+=schermo[i_pac+1][j_pac];
			if(schermo[i_pac+1][j_pac]>0) {
					countPillar--;
					
				}
				if(i_pac!=-1) {
					cancellaPac(i_pac, j_pac); //cancello pacman nella vecchia casella
					
				}
				
			
					i_pac+=1;

					disegnaPacmanRight(i_pac, j_pac);

				
				
				PutChar(160,20,(score/100)%10+48,Red,Black);
				PutChar(168,20,(score/10)%10+48,Red,Black);
				PutChar(152,20,(score/1000)%10+48,Red,Black);
				PutChar(176,20,score%10+48,Red,Black);
				
				if(score!=0 && scoretmp%1000>score%1000 && score!=scoretmp) {
					i_life+=3;
					disegnaPacmanLeftGrande(i_life,j_life);
										current_lives_remaining++;

				}
				
			}
			
			else { //se è un muro disegno pacman right
				cancellaPac(i_pac, j_pac);
				disegnaPacmanRight(i_pac, j_pac);
				
			}
			
		}
	
		
		if(position==3) { //LEFT
			if (i_pac==0) {
				cancellaPac(i_pac, j_pac);
				i_pac=22;
			
			}
			if(schermo[i_pac-1][j_pac]!=-1) { //non è un muro
				score+=schermo[i_pac-1][j_pac];
			if(schermo[i_pac-1][j_pac]>0) {
					countPillar--;
					
				}
				if (i_pac!=22) {
					cancellaPac(i_pac, j_pac); //cancello pacman nella vecchia casella

				}
				
				
				
				i_pac-=1;

				disegnaPacmanLeft(i_pac, j_pac);
				
				PutChar(160,20,(score/100)%10+48,Red,Black);
				PutChar(168,20,(score/10)%10+48,Red,Black);
				PutChar(152,20,(score/1000)%10+48,Red,Black);
				PutChar(176,20,score%10+48,Red,Black);
				
				
				if(score!=0 && scoretmp%1000>score%1000 && score!=scoretmp) {
					i_life+=3;
					disegnaPacmanLeftGrande(i_life,j_life);
										current_lives_remaining++;

				}
			}
			
			else { //se è un muro disegno pacman left
				cancellaPac(i_pac, j_pac);
				disegnaPacmanLeft(i_pac, j_pac);
				
			}
			
		}
		
		
		if(position==4) { //DOWN
			if(schermo[i_pac][j_pac+1]!=-1) { //non è un muro
				score+=schermo[i_pac][j_pac+1];
			if(schermo[i_pac][j_pac+1]>0) {
					countPillar--;
					
				}
				cancellaPac(i_pac, j_pac); //cancello pacman nella vecchia casella
				
				disegnaPacmanDown(i_pac, j_pac+1);
				j_pac+=1;
				
				PutChar(160,20,(score/100)%10+48,Red,Black);
				PutChar(168,20,(score/10)%10+48,Red,Black);
				PutChar(152,20,(score/1000)%10+48,Red,Black);
				PutChar(176,20,score%10+48,Red,Black);
				
				if(score!=0 && scoretmp%1000>score%1000 && score!=scoretmp) {
					i_life+=3;
					disegnaPacmanLeftGrande(i_life,j_life);
										current_lives_remaining++;

				}
				
			}
			
			else { //se è un muro disegno pacman down
				cancellaPac(i_pac, j_pac);
				disegnaPacmanDown(i_pac, j_pac);
				
			}
			
		}
	
	
		
	}
	
	
	

	}
	
	if(dead==1) { //se entro essendo morto pulisco flag
		dead=0;
	}
		
	
	reset_RIT();

	LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
	return;
}

/******************************************************************************
**                            End Of File

******************************************************************************/
