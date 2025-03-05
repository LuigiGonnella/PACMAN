/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           RIT.h
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        Prototypes of functions included in the lib_RIT, funct_RIT, IRQ_RIT .c files
** Correlated files:    lib_RIT.c, funct_RIT.c, IRQ_RIT.c
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#ifndef __RIT_H
#define __RIT_H

#define N1 22
#include <stdio.h>
#include <stdint.h>
#include "CAN/CAN.h"                      /* LPC17xx CAN adaption layer */
#include "music/music.h"

/* init_RIT.c */
extern uint32_t init_RIT( uint32_t RITInterval );
extern void enable_RIT( void );
extern void disable_RIT( void );
extern void reset_RIT( void );
/* IRQ_RIT.c */
extern void RIT_IRQHandler (void);

extern int schermo[N1][N1];
extern int i_pac,j_pac,score,i_life,j_life;
extern void disegnaPacmanLeft(int i, int j);
extern void disegnaPacmanRight(int i, int j);
extern void disegnaPacmanDown(int i, int j);
extern void disegnaPacmanUp(int i, int j);
extern void cancellaPac(int i, int j);
extern void disegnaPacmanLeftGrande(int i, int j);
extern void cancellaPacmanLeftGrande(int i, int j);
extern void disegna(int i, int j);
extern void cancellaFantasma(int i, int j);
extern void disegnaFantasma(int i, int j, int colore);
extern void startGhost();
extern void moveGhost();


extern int countPillar,ticks, current_lives_remaining, flagm;
extern int i_ghost, j_ghost, dead, power,seconds, secondpow, secondspawn;


#endif /* end __RIT_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
