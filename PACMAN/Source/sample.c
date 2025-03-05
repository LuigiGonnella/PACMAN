/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               main.c
** Descriptions:            The GLCD application function
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2010-11-7
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             Paolo Bernardi
** Modified date:           03/01/2020
** Version:                 v2.0
** Descriptions:            basic program for LCD and Touch Panel teaching
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "LPC17xx.h"
#include "GLCD/GLCD.h" 
#include "TouchPanel/TouchPanel.h"
#include "timer/timer.h"
#include "RIT/RIT.h"
#include "joystick/joystick.h"
#include "AsciiLib.h" // Libreria per i caratteri ASCII (opzionale)
#include "button_EXINT/button.h"



#include <stdio.h>
#include <stdbool.h>
#include <limits.h>


// Dimensioni dello schermo
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320
#define N1 22
#define PILLARS 240
int schermo[N1][N1]; //schermo
int countPillar=PILLARS; //numero pillars da generare
//COORDINATE ATTUALI PACMAN
int i_pac=10;
int j_pac=13;

int score=0; //punteggio
//COORDINATE ULTIMA VITA AGGIUNTA
int i_life=0; 
int j_life=23;

volatile int pause; //flag per pausa
int backgroundColor=Yellow;
//COORDINATE ATTUALI FANTASMA
int i_ghost=10;
int j_ghost=10;

int dead=0; //flag morte
int power=0; //flag frightned mode (appena mangio powerpill)
int secondpow=0; //conteggio 10s di frightned mode
int secondspawn=-1; //conteggio 3s per respawn fantasma
int	current_lives_remaining=1; //vite attuali


#ifdef SIMULATOR
extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif

// Prototipi delle funzioni
void drawMaze(void);
void disegna(int i, int j); //disegno schermo iniziale
void inizializzaschermo(void);
void DisegnaLabirinto(void); //disegno labirinto interno
void DisegnaForma(int i, int j, char tipo); //funzione per varie forme nel labirinto
void CancellaCella(int i, int j);
//DISEGNI PER PACMAN NELLA VARIE DIREZIONI
void disegnaPacmanLeft(int i, int j);
void disegnaPacmanRight(int i, int j);
void disegnaPacmanDown(int i, int j);
void disegnaPacmanUp(int i, int j);

void inizializzaPillars(void); //disegno i 240 pillarsS
void disegnaPillarM(int i, int j); //per i pillar grandi
void disegnaPillarS(int i, int j);
void cancellaPac(int i, int j);
void disegnaPacmanLeftGrande(int i, int j); //disegno vita
void cancellaPacmanLeftGrande(int i, int j);
void disegnaFantasma(int i, int j, int colore);
void cancellaFantasma(int i, int j);
void startGhost(); // tolgo cancello fantasma
void moveGhost(); // muovo fantasma






int main(void)
{
	int i,j,c,k;
  SystemInit();  												/* System Initialization (i.e., PLL)  */
	
	
  LCD_Initialization();
	
	LCD_Clear(White);
	GUI_Text(65, 150, (uint8_t *) " PLAY PAC-MAN ", Yellow, Black);
	
	// Disegna il labirinto
	LCD_Clear(Black);
	GUI_Text(8*10+10, 13*10+40, (uint8_t *) " READY! ", Yellow, Black);
	LCD_Clear(Black);
	
	drawMaze(); 
	
	pause=1; //gioco inizia in pausa
	
	BUTTON_init();
	
	init_timer(1, 0, 0, 0x3, 0.25*0x1312D0); // TIMER PER IL COUNTDOWN (0.25*50ms) metto INTERRUPT e RESET per entrare nell'handler ogni 1s 
	init_timer(2, 0, 0, 0x3, 0x1312D0); //TIMER PER MUSICA 
	
	init_timer(2, 0, 1, 0x3, 0x1312D0); // TIMER PER pillarsM CASUALI
	

	
	
	

	
	

	init_RIT(0x1312D0); //(50ms) 1s 
	
		CAN_setup(1);                    // Setup CAN controller 1
    CAN_setup(2);                    // Setup CAN controller 2

    CAN_start(1);                    // Start CAN controller 1
    CAN_start(2);                    // Start CAN controller 2

    CAN_waitReady(1);                // Wait  CAN controller 1 
    CAN_waitReady(2);                // Wait  CAN controller 2 

    NVIC_EnableIRQ(CAN_IRQn);        // Enable CAN interrupt

    CAN_TxMsg.id = 33;               // CAN message ID
    CAN_TxMsg.len = 4;               // CAN message data length



	LPC_PINCON->PINSEL1 |= (1<<21);
	LPC_PINCON->PINSEL1 &= ~(1<<20);
	LPC_GPIO0->FIODIR |= (1<<26);
  
	
	LPC_SC->PCON |= 0x1;									/* power-down	mode										*/
	LPC_SC->PCON &= ~(0x2);						
	
	
  while (1)	
  {
		__ASM("wfi");
  }
}

// Funzione per disegnare un labirinto semplice
void drawMaze(void) {
	
	DisegnaLabirinto();
  return;
}

void disegna(int i, int j) { //i e j sono coordinate fittizie
	if (schermo[i][j]==-1 || i>=N1 || j>=N1) return; //se sono fuori dalla zona di disegno oppure sono già in un muro
	
	int x=i*10+10,y=j*10+40; //x e y sono pixel effettivi
	int k;
		
	for(k=0;k<10;k++) {
		LCD_DrawLine(x,y+k,x+9,y+k,Yellow);

	}


	schermo[i][j]=-1;

	return;
}

void inizializzaschermo(void) {
	int i,j;
	
	for (i=0;i<N1;i++) {
		for (j=0;j<N1;j++) {
			schermo[i][j]=0; //se è casella vuota (avrà 1 se c'è un pillar);
		}
	}
	
	for (i=0;i<N1;i++) { //contorno esterno
		if (i!=10 && i!=11) { //lascio buco per teletrasporto
			disegna(0,i);
		}
		disegna(i,0);
		disegna(i,N1-1);
		if (i!=10 && i!=11) { //lascio buco per teletrasporto
			disegna(N1-1,i);
		}
	}
	
	return;
	
}

void DisegnaLabirinto(void) {
	int i,j;
	
	inizializzaschermo();
	
	DisegnaForma(2,2,'b'); //QUADRANTE 0,0
	DisegnaForma(6,2,'a');
	DisegnaForma(1,4,'a');
	DisegnaForma(8,2,'e');
	DisegnaForma(10,2,'g');
	DisegnaForma(5,4,'d');
	DisegnaForma(3,5,'c');
	DisegnaForma(1,9,'a');
	DisegnaForma(3,9,'f');
	DisegnaForma(9,9,'h');
	
	// Copia speculare negli altri quadranti
    for (i = 0; i < N1 / 2; i++) {
        for (j = 0; j < N1 / 2; j++) {
            if (schermo[i][j] == -1) { // Controlla se è un muro
                // Quadrante in alto a destra
                disegna(i, N1 - 1 - j);
                // Quadrante in basso a sinistra
                disegna(N1 - 1 - i, j);
                // Quadrante in basso a destra
                disegna(N1 - 1 - i, N1 - 1 - j);
            }
        }
    }
		
		CancellaCella(9,2);
		
		
		
			
		
		GUI_Text(9*10+10, 12*10+34, (uint8_t *) "PAUSE", Yellow, Blue);
		
		GUI_Text(20, 5, (uint8_t *) " COUNTDOWN ", Red, Black);
		PutChar(48,20,6+48,Red,Black);
		PutChar(57,20,0+48,Red,Black);
		PutChar(66,20,115,Red,Black);


		
		GUI_Text(150, 5, (uint8_t *) "SCORE", Red, Black);
		PutChar(176,20,(score/10)%10+48,Red,Black);
		PutChar(152,20,(score/100)%10+48,Red,Black);
		PutChar(168,20,score%10+48,Red,Black);
		PutChar(160,20,(score/1000)%10+48,Red,Black);
		
		disegnaPacmanRight(i_pac,j_pac);
		inizializzaPillars();
		
		CancellaCella(10,9);
		CancellaCella(11,9);
		
		LCD_DrawLine(10*10+10,10*10+35,12*10+9,10*10+35,Red);
		disegnaPacmanLeftGrande(i_life,j_life);
		disegnaFantasma(i_ghost,j_ghost, Red);
		
		
		
		
	
	
	return;
	
}

void DisegnaForma(int i, int j, char tipo) { //i,j è il quadrato più in alto a sinistra
	
	switch(tipo) {
		case 'a': //quadrato semplice
			disegna(i,j); 
			break;
		case 'b': //forma a T
			disegna(i,j); 
			disegna(i+1,j);
			disegna(i+2,j);
			disegna(i+3,j);

			disegna(i+1,j+1); 
			break;	
		case 'c': //forma POLLICE
			disegna(i,j);
		  disegna(i,j+1);
			disegna(i-1,j+1);
			disegna(i-1,j+2);
			disegna(i,j+2);
			break;
		case 'd': //forma PISTOLA
			disegna(i,j);
			disegna(i,j+1);
			disegna(i,j+2);
			disegna(i+1,j+2);
			disegna(i,j+3);
			disegna(i+1,j+3);
			disegna(i,j+4);
			break;
		
		case 'e': //forma ASSALTO
			disegna(i,j);
			disegna(i+1,j);

			
		
			disegna(i,j+1);
			disegna(i,j+2);
			disegna(i,j+3);
			disegna(i,j+4);
			disegna(i,j+5);
		  disegna(i-1,j+2);
		  break;

		case 'f': //forma CASTELLO
			disegna(i,j);
			disegna(i,j+1);
			disegna(i+2,j+1);
			disegna(i+4,j+1);
			disegna(i+4,j);
		  break;
		

		
		
		case 'g': //forma LINEA
			disegna(i,j);
			disegna(i,j+2);
			disegna(i,j+3);
			disegna(i,j+5);
      break;
		
		case 'h': //forma ANGOLO
			disegna(i,j);
			disegna(i+1,j);
			disegna(i,j+1);
		  break;
		
		
		
			
		default:
			break;
	}
	return;
}

void CancellaCella(int i, int j) {
	if (i>=N1 || j>=N1) return; //se sono fuori dalla zona di disegno oppure sono già in un muro
	
	int x=i*10+10,y=j*10+40; //x e y sono pixel effettivi
	int k;
	
	for(k=0;k<10;k++) {
		LCD_DrawLine(x,y+k,x+9,y+k,Black);

	}
	schermo[i][j]=0;

	return;

}



void disegnaPillarS(int i, int j){
	schermo[i][j] = 10;
	int x, y;
	x = i*10 + 10;
	y = j*10 + 40;
	
	LCD_DrawLine(x+4, y+3, x+4, y+6, White);
	LCD_DrawLine(x+5, y+3, x+5, y+6, White);
	LCD_DrawLine(x+3, y+4, x+6, y+4, White);
	LCD_DrawLine(x+3, y+5, x+6, y+5, White);

}

void disegnaPillarM(int i, int j){
	schermo[i][j] = 50;
	int x, y;
	x = i*10 + 10;
	y = j*10 + 40;
	LCD_DrawLine(x+2, y+4, x+7, y+4, Orange);
	LCD_DrawLine(x+2, y+5, x+7, y+5, Orange);
	
	LCD_DrawLine(x+4, y+2, x+4, y+7, Orange);	
	LCD_DrawLine(x+5, y+2, x+5, y+7, Orange);
	
	LCD_DrawLine(x+3, y+3, x+6, y+3, Orange);	
	LCD_DrawLine(x+3, y+6, x+6, y+6, Orange);	
}

void disegnaPillarXL(int i, int j){
	schermo[i][j] = 100;
	int x, y;
	x = i*10 + 10;
	y = j*10 + 40;
	LCD_DrawLine(x+4, y+1, x+5, y+1, Green);
	LCD_DrawLine(x+3, y+2, x+6, y+2, Green);
	LCD_DrawLine(x+2, y+3, x+7, y+3, Green);
	LCD_DrawLine(x+1, y+4, x+8, y+4, Green);
	
	LCD_DrawLine(x+1, y+5, x+8, y+5, Green);
	LCD_DrawLine(x+2, y+6, x+7, y+6, Green);
	LCD_DrawLine(x+3, y+7, x+6, y+7, Green);
	LCD_DrawLine(x+4, y+8, x+5, y+8, Green);

}

void inizializzaPillars(){
	int i, j;
	volatile int i_rand = 0, j_rand = 0;
	
	for(i = 0; i < N1; i++){
		for(j = 0; j < N1; j++){
			if(schermo[i][j] == 0 && (i != i_pac || j != j_pac) && (i != N1/2 - 1 || j != N1/2 -1) && (i != N1/2 - 1 || j != N1/2) && (i != N1/2 || j != N1/2 -1) && (i != N1/2|| j != N1/2)){
				disegnaPillarS(i, j);
			}
		}
	}
		


}

void disegnaPacmanLeft(int i, int j){ // coordinate fittizie della mia matrice 20x20
	if(i >= N1 || j >= N1) return;
	int x = i*10+10, y = j*10+40;					// pixel effettivi con offset (laterale e verticale per avere bene lo schermo)
	
		LCD_DrawLine(x+3, y, x+6, y, Yellow);
		LCD_DrawLine(x+2, y+1, x+7, y+1, Yellow);
		LCD_DrawLine(x+1, y+2, x+8, y+2, Yellow);
		LCD_DrawLine(x+2, y+3, x+9, y+3, Yellow);
		LCD_DrawLine(x+4, y+4, x+9, y+4, Yellow);
		LCD_DrawLine(x+6, y+5, x+9, y+5, Yellow);
		LCD_DrawLine(x+4, y+6, x+9, y+6, Yellow);
		LCD_DrawLine(x+2, y+7, x+9, y+7, Yellow);
		LCD_DrawLine(x+1, y+8, x+8, y+8, Yellow);
		LCD_DrawLine(x+2, y+9, x+7, y+9, Yellow);
	
	
	return;
}

void disegnaPacmanRight(int i, int j){ // coordinate fittizie della matrice 20x20
	if(i >= N1 || j >= N1) return;
	int x = i*10+10, y = j*10+40; // pixel effettivi con offset (laterale e verticale per avere bene lo schermo)
	
		LCD_DrawLine(x+4, y, x+7, y, Yellow);
		LCD_DrawLine(x+3, y+1, x+8, y+1, Yellow);
		LCD_DrawLine(x+2, y+2, x+9, y+2, Yellow);
		LCD_DrawLine(x+1, y+3, x+8, y+3, Yellow);
		LCD_DrawLine(x+1, y+4, x+6, y+4, Yellow);
		LCD_DrawLine(x+1, y+5, x+4, y+5, Yellow);
		LCD_DrawLine(x+1, y+6, x+6, y+6, Yellow);
		LCD_DrawLine(x+1, y+7, x+8, y+7, Yellow);
		LCD_DrawLine(x+2, y+8, x+9, y+8, Yellow);
		LCD_DrawLine(x+3, y+9, x+8, y+9, Yellow);
	
	return;
}


void disegnaPacmanUp(int i, int j){ // Bocca rivolta verso l'alto
	if(i >= N1 || j >= N1) return;
	int x = i*10+10, y = j*10+40;
	
		LCD_DrawLine(x, y+3, x, y+6, Yellow);
		LCD_DrawLine(x+1, y+2, x+1, y+7, Yellow);
		LCD_DrawLine(x+2, y+1, x+2, y+8, Yellow);
		LCD_DrawLine(x+3, y+2, x+3, y+9, Yellow);
		LCD_DrawLine(x+4, y+4, x+4, y+9, Yellow);
		LCD_DrawLine(x+5, y+6, x+5, y+9, Yellow);
		LCD_DrawLine(x+6, y+4, x+6, y+9, Yellow);
		LCD_DrawLine(x+7, y+2, x+7, y+9, Yellow);
		LCD_DrawLine(x+8, y+1, x+8, y+8, Yellow);
		LCD_DrawLine(x+9, y+2, x+9, y+7, Yellow);
	
	return;
}

void disegnaPacmanDown(int i, int j){ // Bocca rivolta verso il basso
	if(i >= N1 || j >= N1) return;
	int x = i*10+10, y = j*10+40;
	
		LCD_DrawLine(x, y+4, x, y+7, Yellow);
		LCD_DrawLine(x+1, y+3, x+1, y+8, Yellow);
		LCD_DrawLine(x+2, y+2, x+2, y+9, Yellow);
		LCD_DrawLine(x+3, y+1, x+3, y+8, Yellow);
		LCD_DrawLine(x+4, y+1, x+4, y+6, Yellow);
		LCD_DrawLine(x+5, y+1, x+5, y+4, Yellow);
		LCD_DrawLine(x+6, y+1, x+6, y+6, Yellow);
		LCD_DrawLine(x+7, y+1, x+7, y+8, Yellow);
		LCD_DrawLine(x+8, y+2, x+8, y+9, Yellow);
		LCD_DrawLine(x+9, y+3, x+9, y+8, Yellow);
	
	
	return;
}


void cancellaPac(int i, int j) {
	if(i >= N1 || j >= N1) return;
	int x = i*10+10, y = j*10+40;
	LCD_DrawLine(x, y, x+9, y, Black);
	LCD_DrawLine(x, y+1, x+9, y+1, Black);
	LCD_DrawLine(x, y+2, x+9, y+2, Black);
	LCD_DrawLine(x, y+3, x+9, y+3, Black);
	LCD_DrawLine(x, y+4, x+9, y+4, Black);
	LCD_DrawLine(x, y+5, x+9, y+5, Black);
	LCD_DrawLine(x, y+6, x+9, y+6, Black);
	LCD_DrawLine(x, y+7, x+9, y+7, Black);
	LCD_DrawLine(x, y+8, x+9, y+8, Black);
	LCD_DrawLine(x, y+9, x+9, y+9, Black);
	
	if(schermo[i][j]==50) {
		power=1;
		secondpow=seconds-10;
	}

	schermo[i][j]=0;
	
	return;
}

void disegnaPacmanLeftGrande(int i, int j){ // coordinate della matrice 20x20
    int x = i * 10 + 10; // Offset laterale per schermo
    int y = j * 10 + 40; // Offset verticale per schermo

    // Disegno scalato: ogni coordinata moltiplicata per 4
	
	
		LCD_DrawLine(x+3, y+1, x+14, y+1, Yellow);

		LCD_DrawLine(x+2, y+2, x+15, y+2, Yellow);
		LCD_DrawLine(x+2, y+3, x+15, y+3, Yellow);
	
		LCD_DrawLine(x+1, y+4, x+17, y+4, Yellow);
		LCD_DrawLine(x+1, y+5, x+17, y+5, Yellow);
	
		LCD_DrawLine(x+4, y+6, x+18, y+6, Yellow);
		LCD_DrawLine(x+4, y+7, x+18, y+7, Yellow);
	
		LCD_DrawLine(x+9, y+8, x+18, y+8, Yellow);
		LCD_DrawLine(x+9, y+9, x+18, y+9, Yellow);
	
		LCD_DrawLine(x+13, y+10, x+18, y+10, Yellow);
		LCD_DrawLine(x+13, y+11, x+18, y+11, Yellow);
		
		LCD_DrawLine(x+9, y+12, x+18, y+12, Yellow);
		LCD_DrawLine(x+9, y+13, x+18, y+13, Yellow);
		
		LCD_DrawLine(x+4, y+14, x+18, y+14, Yellow);
		LCD_DrawLine(x+4, y+15, x+18, y+15, Yellow);
		
		LCD_DrawLine(x+1, y+16, x+17, y+16, Yellow);
		LCD_DrawLine(x+1, y+17, x+17, y+17, Yellow);
		
		LCD_DrawLine(x+2, y+18, x+15, y+18, Yellow);
		LCD_DrawLine(x+2, y+19, x+15, y+19, Yellow);
		
		LCD_DrawLine(x+3, y+20, x+14, y+20, Yellow);
		
		
		return;
}


void cancellaPacmanLeftGrande(int i, int j){ // coordinate della matrice 20x20
    int x = i * 10 + 10; // Offset laterale per schermo
    int y = j * 10 + 40; // Offset verticale per schermo

    // Disegno scalato: ogni coordinata moltiplicata per 4
	
	
		LCD_DrawLine(x+3, y+1, x+14, y+1, Black);

		LCD_DrawLine(x+2, y+2, x+15, y+2, Black);
		LCD_DrawLine(x+2, y+3, x+15, y+3, Black);
	
		LCD_DrawLine(x+1, y+4, x+17, y+4, Black);
		LCD_DrawLine(x+1, y+5, x+17, y+5, Black);
	
		LCD_DrawLine(x+4, y+6, x+18, y+6, Black);
		LCD_DrawLine(x+4, y+7, x+18, y+7, Black);
	
		LCD_DrawLine(x+9, y+8, x+18, y+8, Black);
		LCD_DrawLine(x+9, y+9, x+18, y+9, Black);
	
		LCD_DrawLine(x+13, y+10, x+18, y+10, Black);
		LCD_DrawLine(x+13, y+11, x+18, y+11, Black);
		
		LCD_DrawLine(x+9, y+12, x+18, y+12, Black);
		LCD_DrawLine(x+9, y+13, x+18, y+13, Black);
		
		LCD_DrawLine(x+4, y+14, x+18, y+14, Black);
		LCD_DrawLine(x+4, y+15, x+18, y+15, Black);
		
		LCD_DrawLine(x+1, y+16, x+17, y+16, Black);
		LCD_DrawLine(x+1, y+17, x+17, y+17, Black);
		
		LCD_DrawLine(x+2, y+18, x+15, y+18, Black);
		LCD_DrawLine(x+2, y+19, x+15, y+19, Black);
		
		LCD_DrawLine(x+3, y+20, x+14, y+20, Black);
		
		
		return;
}

void disegnaFantasma(int i, int j, int colore){
		int x = i*10+10;
	  int y = j*10+40;
	  if(schermo[i][j] == -1)
			return;
		//corpo
		LCD_DrawLine(x+2, y, x+7, y, colore);	
		LCD_DrawLine(x+1, y+1, x+8, y+1, colore);	
		LCD_DrawLine(x, y+2, x, y+7, colore);	
		LCD_DrawLine(x+9, y+2, x+9, y+7, colore);	
		LCD_DrawLine(x+8, y+4, x+9, y+4, colore);	
		LCD_DrawLine(x+1, y+2, x+1, y+2, colore);	
		LCD_DrawLine(x+8, y+2, x+8, y+2, colore);	
		LCD_DrawLine(x+3, y+2, x+6, y+2, colore);	
		LCD_DrawLine(x+4, y+3, x+5, y+3, colore);
	  LCD_DrawLine(x+3, y+4, x+6, y+4, colore);	
		LCD_DrawLine(x, y+4, x+1, y+4, colore);
		LCD_DrawLine(x, y+5, x+9, y+5, colore);
		LCD_DrawLine(x, y+6, x+9, y+6, colore);
		LCD_DrawLine(x, y+7, x+9, y+7, colore);
		LCD_DrawLine(x+1, y+8, x+1, y+9, colore);
		LCD_DrawLine(x+3, y+8, x+3, y+9, colore);
		LCD_DrawLine(x+6, y+8, x+6, y+9, colore);
		LCD_DrawLine(x+8, y+8, x+8, y+9, colore);
		
		//occhi
		LCD_DrawLine(x+1, y+3, x+3, y+3, White);
		LCD_DrawLine(x+2, y+2, x+2, y+4, White);
		LCD_DrawLine(x+6, y+3, x+8, y+3, White);
		LCD_DrawLine(x+7, y+2, x+7, y+4, White);

		LCD_DrawLine(x+2, y+3, x+2, y+3, Black);
		LCD_DrawLine(x+7, y+3, x+7, y+3, Black);
		return;
}

void cancellaFantasma(int i, int j){
	if(i >= N1 || j >= N1) return;
	int x = i*10+10, y = j*10+40,k;
	
	
	for(k=0;k<10;k++) {
		LCD_DrawLine(x,y+k,x+9,y+k,Black);

	}
	
	if(schermo[i][j] == 10)
		disegnaPillarS(i, j);
	else if(schermo[i][j] == 50)
		disegnaPillarM(i, j);
	return;
}

void startGhost() {
	LCD_DrawLine(10*10+10,10*10+35,12*10+9,10*10+35,Black);
	schermo[10][9]=0;
	schermo[11][9]=0;
}

void moveGhost() {
	
	
	
	if(secondspawn==-1) {
		cancellaFantasma(i_ghost,j_ghost);

	

	if(power==0 ) { //FANTASMA ATTACCA
		
		
		if(schermo[i_ghost][j_ghost]==10) {
		disegnaPillarS(i_ghost,j_ghost);
	}
	else if(schermo[i_ghost][j_ghost]==50) {
		disegnaPillarM(i_ghost,j_ghost);
	}
	if(i_pac>i_ghost && schermo[i_ghost+1][j_ghost]!=-1 && i_ghost+1<N1) {
		i_ghost++;
	}
	else if(i_pac<i_ghost && schermo[i_ghost-1][j_ghost]!=-1 && i_ghost-1>=0) {
		i_ghost--;
	}
	else if(j_pac>j_ghost && schermo[i_ghost][j_ghost+1]!=-1 && j_ghost+1<N1) {
		j_ghost++;
	}
	else if(j_pac<j_ghost && schermo[i_ghost][j_ghost-1]!=-1 && j_ghost-1>=0) {
		j_ghost--;
	}
	else if(schermo[i_ghost+1][j_ghost]!=-1 && i_ghost+1<N1) {
		i_ghost++;
	}
	else if(schermo[i_ghost-1][j_ghost]!=-1 && i_ghost-1>=0) {
		i_ghost--;
	}
	else if(schermo[i_ghost][j_ghost+1]!=-1 && j_ghost+1<N1) {
		j_ghost++;
	}
	else if(schermo[i_ghost][j_ghost-1]!=-1 && j_ghost-1>=0) {
		j_ghost--;
	}
	
	
		disegnaFantasma(i_ghost,j_ghost,Red);

	
		
	}
	
	else { //PACMAN ATTACCA
		int i_target, j_target;
		
		
		if (i_pac>=10) {
			i_target=1;
		}
		else {
			i_target=20;
		}
		
		if (j_pac>=10) {
			j_target=1;
		}
		else {
			j_target=20;
		}
		
		if(schermo[i_ghost][j_ghost]==10) {
		disegnaPillarS(i_ghost,j_ghost);
	}
	else if(schermo[i_ghost][j_ghost]==50) {
		disegnaPillarM(i_ghost,j_ghost);
	}
	if(i_target>i_ghost && schermo[i_ghost+1][j_ghost]!=-1 && i_ghost+1<N1) {
		i_ghost++;
	}
	else if(i_target<i_ghost && schermo[i_ghost-1][j_ghost]!=-1 && i_ghost-1>=0) {
		i_ghost--;
	}
	else if(j_target>j_ghost && schermo[i_ghost][j_ghost+1]!=-1 && j_ghost+1<N1) {
		j_ghost++;
	}
	else if(j_target<j_ghost && schermo[i_ghost][j_ghost-1]!=-1 && j_ghost-1>=0) {
		j_ghost--;
	}
	else if(schermo[i_ghost+1][j_ghost]!=-1 && i_ghost+1<N1) {
		i_ghost++;
	}
	else if(schermo[i_ghost-1][j_ghost]!=-1 && i_ghost-1>=0) {
		i_ghost--;
	}
	else if(schermo[i_ghost][j_ghost+1]!=-1 && j_ghost+1<N1) {
		j_ghost++;
	}
	else if(schermo[i_ghost][j_ghost-1]!=-1 && j_ghost-1>=0) {
		j_ghost--;
	}
	
	
				
			disegnaFantasma(i_ghost,j_ghost,Blue);
		

		
	
	
	
	
	}
	
	}
	

	


}



/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
