/************************************************************************/
/*																		*/
/*	delay.c	--	Time Delay Functions									*/
/*																		*/
/************************************************************************/
/*	Author: 	Gene Apperson											*/
/*			Convert to StellarisWare - David Glover 04/09/2014					*/
/*	Copyright 2011, Digilent Inc.										*/
/************************************************************************/
/*  Module Description: 												*/
/*																		*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/*	04/29/2011(GeneA): created	
/	07/09/2014(DavidG): Ported to StellarisWare.
/			    Replaced timer implementation with 
/			    SysCtrlDelay() call.
/									*/
/*																		*/
/************************************************************************/


/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */
#include <stdint.h>
#include <stdbool.h>
//#include <p32xxxx.h>
//#include <plib.h>
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"

#include "delay.h"

/* ------------------------------------------------------------ */
/*				Local Type Definitions							*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*				Global Variables								*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*				Local Variables									*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*				Forward Declarations							*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*				Procedure Definitions							*/
/* ------------------------------------------------------------ */
/***	DelayInit
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		No initialization needed for Stellarisware SysCtlDelay()
*/

void
DelayInit()	
	{
	//unsigned int tcfg;

	/* Configure Timer 1. This sets it up to count a 10Mhz.
	*/
	//tcfg = T1_ON|T1_IDLE_CON|T1_SOURCE_INT|T1_PS_1_8|T1_GATE_OFF|T1_SYNC_EXT_OFF;
	//OpenTimer1(tcfg, 0xFFFF);

}

/* ------------------------------------------------------------ */
/***	DelayMs
**
**	Parameters:
**		cms			- number of milliseconds to delay
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Delay the requested number of milliseconds. Uses SysCtlDelay().
*/

void
DelayMs(int cms)
	{
	unsigned long ulCount;
	// number of delay loops per ms = frequency / (1000 * 3)
	ulCount = (ROM_SysCtlClockGet() / (1000 * 3)) * cms;
	ROM_SysCtlDelay(ulCount);

}		

