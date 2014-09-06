/************************************************************************/
/*																		*/
/*	oled.h	--	Interface Declarations for OLED Display Driver Module	*/
/*																		*/
/************************************************************************/
/*	Author:		Gene Apperson											*/
/*	Copyright 2011, Digilent Inc.										*/
/************************************************************************/
/*  File Description:													*/
/*																		*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/*	04/29/2011(GeneA): created	
	07/09/02014(DavidG): Changed port / bit definitions for Stellaris 		*/
/*																		*/
/************************************************************************/

#if !defined(PMODOLED_INC)
#define	PMODOLED_INC

/* ------------------------------------------------------------ */
/*					Miscellaneous Declarations					*/
/* ------------------------------------------------------------ */

#define	cbOledDispMax	512		//max number of bytes in display buffer

#define	ccolOledMax		128		//number of display columns
#define	crowOledMax		32		//number of display rows
#define	cpagOledMax		4		//number of display memory pages

#define	cbOledChar		8		//font glyph definitions is 8 bytes long
#define	chOledUserMax	0x20	//number of character defs in user font table
#define	cbOledFontUser	(chOledUserMax*cbOledChar)

/* Graphics drawing modes.
*/
#define	modOledSet		0
#define	modOledOr		1
#define	modOledAnd		2
#define	modOledXor		3

/* ------------------------------------------------------------ */
/*					General Type Declarations					*/
/* ------------------------------------------------------------ */

/* Pin definitions for access to OLED control signals on Stellaris Launchpad
*/
#define	prtDataCmd	GPIO_PORTB_BASE
#define	prtReset	GPIO_PORTB_BASE
#define	prtVbatCtrl 	GPIO_PORTB_BASE
#define	prtVddCtrl	GPIO_PORTB_BASE

#define bitDataCmd	GPIO_PIN_4
#define	bitReset	GPIO_PIN_5
#define	bitVbatCtrl	GPIO_PIN_1
#define	bitVddCtrl	GPIO_PIN_0

/* ------------------------------------------------------------ */
/*					Object Class Declarations					*/
/* ------------------------------------------------------------ */



/* ------------------------------------------------------------ */
/*					Variable Declarations						*/
/* ------------------------------------------------------------ */



/* ------------------------------------------------------------ */
/*					Procedure Declarations						*/
/* ------------------------------------------------------------ */

void	OledInit();
void	OledDevOff();
void	OledClear();
void	OledClearBuffer();
void	OledUpdate();

/* ------------------------------------------------------------ */

#endif

/************************************************************************/
