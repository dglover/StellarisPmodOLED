/************************************************************************/
/*																		*/
/*	oled.c	--	Graphics Driver Library for OLED Display				*/
/*																					*/
/************************************************************************/
/*	Author: 	Gene Apperson											*/
/*	Copyright 2011, Digilent Inc.										*/
/************************************************************************/
/*  Module Description: 												*/
/*																		*/	
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/*	04/29/2011(GeneA): Created											
	07/09/2014(DavidG): Modified to run on Stellaris Launchpad using 
			StellarisWare.								*/
/*																			*/
/************************************************************************/


/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"

#include "PmodOLED.h"
#include "OledChar.h"
#include "OledGrph.h"

#include "delay.h"

/* ------------------------------------------------------------ */
/*				Local Type Definitions							*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*				Global Variables								*/
/* ------------------------------------------------------------ */

extern const unsigned char		rgbOledFont0[];
extern unsigned char		rgbOledFontUser[];
extern const unsigned char		rgbFillPat[];

extern int		xchOledMax;
extern int		ychOledMax;

/* Coordinates of current pixel location on the display. The origin
** is at the upper left of the display. X increases to the right
** and y increases going down.
*/
int		xcoOledCur;
int		ycoOledCur;

unsigned char *	pbOledCur;			//address of unsigned char corresponding to current location
int		bnOledCur;			//bit number of bit corresponding to current location
unsigned char	clrOledCur;			//drawing color to use
unsigned char *	pbOledPatCur;		//current fill pattern
int		fOledCharUpdate;

int		dxcoOledFontCur;
int		dycoOledFontCur;

unsigned char *	pbOledFontCur;
unsigned char *	pbOledFontUser;

/* ------------------------------------------------------------ */
/*				Local Variables									*/
/* ------------------------------------------------------------ */

/* This array is the offscreen frame buffer used for rendering.
** It isn't possible to read back frome the OLED display device,
** so display data is rendered into this offscreen buffer and then
** copied to the display.
*/
unsigned char	rgbOledBmp[cbOledDispMax];

/* ------------------------------------------------------------ */
/*				Forward Declarations							*/
/* ------------------------------------------------------------ */

void	OledHostInit();
void	OledDevInit();
void	OledDvrInit();
void	OledPutBuffer(int cb, unsigned char * rgbTx);

/* ------------------------------------------------------------ */
/*				Procedure Definitions							*/
/* ------------------------------------------------------------ */
/***	OledInit
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
**		Initialize the OLED display subsystem.
*/

void
OledInit()
	{

	/* Init the Stellaris peripherals used to talk to the display.
	*/
	OledHostInit();

	/* Init the memory variables used to control access to the
	** display.
	*/
	OledDvrInit();

	/* Init the OLED display hardware.
	*/
	OledDevInit();

	/* Clear the display.
	*/
	OledClear();

}

/* ------------------------------------------------------------ */
/***	OledHostInit
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
**		Perform Stellaris device initialization to prepare for use
**		of the OLED display.
**		This is currently hard coded for the Cerebot 32MX4 and
**		SPI2. This needs to be generalized.
*/

void
OledHostInit()
	{

	//
	// The SSI3 peripheral must be enabled for use.
	//
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);

	// Enable GPIO port D which is used for SSI3 pins.
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

	//
	// Configure the pin muxing for SSI3 functions on port D0, D1, D2, and D3.
	ROM_GPIOPinConfigure(GPIO_PD0_SSI3CLK);
	ROM_GPIOPinConfigure(GPIO_PD1_SSI3FSS);
	ROM_GPIOPinConfigure(GPIO_PD2_SSI3RX);
	ROM_GPIOPinConfigure(GPIO_PD3_SSI3TX);	

	//
	// Configure the GPIO settings for the SSI pins.  This function also gives
	// control of these pins to the SSI hardware.  Consult the data sheet to
	// see which functions are allocated per pin.
	// The pins are assigned as follows:
	//      PD3 - SSI3Tx
	//      PD2 - SSI3Rx
	//      PD1 - SSI3Fss
	//      PD0 - SSI3CLK
	ROM_GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 |
                   GPIO_PIN_0);
		
	//
	// Configure and enable the SSI port for SPI master mode.  Use SSI3,
	// system clock supply, idle clock level low and active low clock in
	// freescale SPI mode, master mode, 2MHz SSI frequency, and 8-bit data.
	// For SPI mode, you can set the polarity of the SSI clock when the SSI
	// unit is idle.  You can also configure what clock edge you want to
	// capture data on.  Please reference the datasheet for more information on
	// the different SPI modes.
	//
	ROM_SSIConfigSetExpClk(SSI3_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, 2000000, 8);

	//
	// Enable the SSI3 module.
	//
	ROM_SSIEnable(SSI3_BASE);
    		
/*	SPI2CON = 0;
	SPI2BRG = 15;				//8Mhz, with 80Mhz PB clock
	SPI2STATbits.SPIROV = 0;
	SPI2CONbits.CKP = 1;
	SPI2CONbits.MSTEN = 1;
	SPI2CONbits.ON = 1;
*/
	/* Make power control pins be outputs with the supplies off
	*/
	// Enable GPIO port B which is used for power supplies.
	// 	PB0 = VDD power control
	//	PB1 = VBAT power control
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	ROM_GPIOPinTypeGPIOOutput(prtVbatCtrl, bitVbatCtrl | bitVddCtrl);
	ROM_GPIOPinWrite(prtVbatCtrl, bitVbatCtrl | bitVddCtrl, bitVbatCtrl | bitVddCtrl);

/*
	PORTSetBits(prtVddCtrl, bitVddCtrl);
	PORTSetBits(prtVbatCtrl, bitVbatCtrl);
	PORTSetPinsDigitalOut(prtVddCtrl, bitVddCtrl);		//VDD power control (1=off)
	PORTSetPinsDigitalOut(prtVbatCtrl, bitVbatCtrl);	//VBAT power control (1=off)
*/
	/* Make the Data/Command select and Reset pins be outputs.
	*/
	// 	PB4 = Data Command
	//	PB5 = Reset
	//ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);  // already enabled
	ROM_GPIOPinTypeGPIOOutput(prtDataCmd, bitDataCmd | bitReset);
	ROM_GPIOPinWrite(prtDataCmd, bitDataCmd | bitReset, bitDataCmd | bitReset);
	
	
/*	PORTSetBits(prtDataCmd, bitDataCmd);
	PORTSetPinsDigitalOut(prtDataCmd, bitDataCmd);		//Data/Command# select
	PORTSetBits(prtReset, bitReset);
	PORTSetPinsDigitalOut(prtReset, bitReset);
	PORTSetBits(prtSelect, bitSelect);
	PORTSetPinsDigitalOut(prtSelect, bitSelect);
*/
}

/* ------------------------------------------------------------ */
/***	OledDvrInit
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
**		Initialize the OLED software system
*/

void
OledDvrInit()
	{
	int		ib;

	/* Init the parameters for the default font
	*/
	dxcoOledFontCur = cbOledChar;
	dycoOledFontCur = 8;
	pbOledFontCur = rgbOledFont0;
	pbOledFontUser = rgbOledFontUser;

	for (ib = 0; ib < cbOledFontUser; ib++) {
		rgbOledFontUser[ib] = 0;
	}

	xchOledMax = ccolOledMax / dxcoOledFontCur;
	ychOledMax = crowOledMax / dycoOledFontCur;

	/* Set the default character cursor position.
	*/
	OledSetCursor(0, 0);

	/* Set the default foreground draw color and fill pattern
	*/
	clrOledCur = 0x01;
	pbOledPatCur = rgbFillPat;
	OledSetDrawMode(modOledSet);

	/* Default the character routines to automaticall
	** update the display.
	*/
	fOledCharUpdate = 1;

}

/* ------------------------------------------------------------ */
/***	OledDevInit
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
**		Initialize the OLED display controller and turn the display on.
*/

void
OledDevInit()
	{

	// wait until all bytes have been transferred.  Should not be needed for DevInit, 
	// but placed here for consistency.  (We don't want to change to DataCmd value 
	// if we have bytes in the fifo).
	while (ROM_SSIBusy(SSI3_BASE)) 
	{
	}

	/* We're going to be sending commands, so clear the Data/Cmd bit
	*/
	ROM_GPIOPinWrite(prtDataCmd, bitDataCmd, 0);
//	PORTClearBits(prtDataCmd, bitDataCmd);

	/* Start by turning VDD on and wait a while for the power to come up.
	*/
	ROM_GPIOPinWrite(prtVddCtrl, bitVddCtrl, 0);
//	PORTClearBits(prtVddCtrl, bitVddCtrl);
	DelayMs(1);

	/* Display off command
	*/
	ROM_SSIDataPut(SSI3_BASE, 0xAE);
//	Spi2Putunsigned char(0xAE);

	// wait until all bytes have been transferred.
	while (ROM_SSIBusy(SSI3_BASE)) 
	{
	}

	/* Bring Reset low and then high
	*/
	ROM_GPIOPinWrite(prtReset, bitReset, 0);
//	PORTClearBits(prtReset, bitReset);
	DelayMs(1);
	ROM_GPIOPinWrite(prtReset, bitReset, bitReset);
//	PORTSetBits(prtReset, bitReset);

	/* Send the Set Charge Pump and Set Pre-Charge Period commands
	*/
	ROM_SSIDataPut(SSI3_BASE, 0x8D);	
	ROM_SSIDataPut(SSI3_BASE, 0x14);
	
	ROM_SSIDataPut(SSI3_BASE, 0xD9);
	ROM_SSIDataPut(SSI3_BASE, 0xF1);
	
	// wait until all bytes have been transferred.
	while (ROM_SSIBusy(SSI3_BASE)) 
	{
	}

//	Spi2Putunsigned char(0x8D);
//	Spi2Putunsigned char(0x14);

//	Spi2Putunsigned char(0xD9);
//	Spi2Putunsigned char(0xF1);

	/* Turn on VBat and wait 100ms
	*/
	ROM_GPIOPinWrite(prtVbatCtrl, bitVbatCtrl, 0);
//	PORTClearBits(prtVbatCtrl, bitVbatCtrl);
	DelayMs(100);

	/* Send the commands to invert the display.
	*/
	ROM_SSIDataPut(SSI3_BASE, 0xA1);			//remap columns
	ROM_SSIDataPut(SSI3_BASE, 0xC8);			//remap the rows
	
//	Spi2Putunsigned char(0xA1);			//remap columns
//	Spi2Putunsigned char(0xC8);			//remap the rows

	/* Send the commands to select sequential COM configuration
	*/
	ROM_SSIDataPut(SSI3_BASE, 0xDA);			//set COM configuration command
	ROM_SSIDataPut(SSI3_BASE, 0x20);			//sequential COM, left/right remap enabled
	
//	Spi2Putunsigned char(0xDA);			//set COM configuration command
//	Spi2Putunsigned char(0x20);			//sequential COM, left/right remap enabled

	/* Send Display On command
	*/
	ROM_SSIDataPut(SSI3_BASE, 0xAF);
//	Spi2Putunsigned char(0xAF);

}


void
OledDevOff()
	{

	// wait until all bytes have been transferred.
	while (ROM_SSIBusy(SSI3_BASE)) 
	{
	}

	/* We're going to be sending commands, so clear the Data/Cmd bit
	*/
	ROM_GPIOPinWrite(prtDataCmd, bitDataCmd, 0);
//	PORTClearBits(prtDataCmd, bitDataCmd);

	/* Display off command
	*/
	ROM_SSIDataPut(SSI3_BASE, 0xAE);
	
	// wait until all bytes have been transferred.
	while (ROM_SSIBusy(SSI3_BASE)) 
	{
	}

	/* Turn off VCC and wait 100ms
	*/
	ROM_GPIOPinWrite(prtVbatCtrl, bitVbatCtrl, 1);
	DelayMs(100);	
	
	/* Turn off Vdd*/
	ROM_GPIOPinWrite(prtVddCtrl, bitVddCtrl, 0);	
	

}


/* ------------------------------------------------------------ */
/***	OledClear
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
**		Clear the display. This clears the memory buffer and then
**		updates the display.
*/

void
OledClear()
	{

	OledClearBuffer();
	OledUpdate();

}

/* ------------------------------------------------------------ */
/***	OledClearBuffer
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
**		Clear the display memory buffer.
*/

void
OledClearBuffer()
	{
	int			ib;
	unsigned char *		pb;

	pb = rgbOledBmp;

	/* Fill the memory buffer with 0.
	*/
	for (ib = 0; ib < cbOledDispMax; ib++) {
		*pb++ = 0x00;
	}

}

/* ------------------------------------------------------------ */
/***	OledUpdate
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
**		Update the OLED display with the contents of the memory buffer
*/

void
OledUpdate()
	{
	int		ipag;
	int		icol;
	unsigned char *	pb;

	pb = rgbOledBmp;

	for (ipag = 0; ipag < cpagOledMax; ipag++) {

		// wait until all bytes have been transferred.
		while (ROM_SSIBusy(SSI3_BASE)) 
		{
		}

		ROM_GPIOPinWrite(prtDataCmd, bitDataCmd, 0);
//		PORTClearBits(prtDataCmd, bitDataCmd);

		/* Set the page address
		*/
		ROM_SSIDataPut(SSI3_BASE, 0x22);	// Set page command
		ROM_SSIDataPut(SSI3_BASE, ipag);	// page number

//		Spi2Putunsigned char(0x22);		//Set page command
//		Spi2Putunsigned char(ipag);		//page number

		/* Start at the left column
		*/
		ROM_SSIDataPut(SSI3_BASE, 0x00);	//set low nybble of column
		ROM_SSIDataPut(SSI3_BASE, 0x10);	// set high nybble of column
		
//		Spi2Putunsigned char(0x00);		//set low nybble of column
//		Spi2Putunsigned char(0x10);		//set high nybble of column

		// wait until all bytes have been transferred.
		while (ROM_SSIBusy(SSI3_BASE)) 
		{
		}

		ROM_GPIOPinWrite(prtDataCmd, bitDataCmd, bitDataCmd);
//		PORTSetBits(prtDataCmd, bitDataCmd);

		/* Copy this memory page of display data.
		*/
		OledPutBuffer(ccolOledMax, pb);
		pb += ccolOledMax;

	}

}

/* ------------------------------------------------------------ */
/***	OledPutBuffer
**
**	Parameters:
**		cb		- number of unsigned chars to send/receive
**		rgbTx	- pointer to the buffer to send
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Send the unsigned chars specified in rgbTx to the slave
*/

void
OledPutBuffer(int cb, unsigned char * rgbTx)
	{
	int		ib;
	unsigned char	bTmp;

	/* Bring the slave select line low
	*/
//	PORTClearBits(prtSelect, bitSelect);

	/* Write/Read the data
	*/
	for (ib = 0; ib < cb; ib++) {
		/* Wait for transmitter to be ready
		*/
//		while (SPI2STATbits.SPITBE == 0);

		/* Write the next transmit unsigned char.
		*/
		ROM_SSIDataPut(SSI3_BASE, *rgbTx++);
//		SPI2BUF = *rgbTx++;

		/* Wait for receive unsigned char.
		*/
//		while (SPI2STATbits.SPIRBF == 0);
//		bTmp = SPI2BUF;

	}

	/* Bring the slave select line high
	*/
//	PORTSetBits(prtSelect, bitSelect);
	
}

/* ------------------------------------------------------------ */
/***	Spi2Putunsigned char
**
**	Parameters:
**		bVal		- unsigned char value to write
**
**	Return Value:
**		Returns unsigned char read
**
**	Errors:
**		none
**
**	Description:
**		Write/Read a unsigned char on SPI port 2
*/

//unsigned char
//Spi2Putunsigned char(unsigned char bVal)
//	{
//	unsigned char	bRx;
//
//	/* Bring the slave select line low
//	*/
//	PORTClearBits(prtSelect, bitSelect);
//
//	/* Wait for transmitter to be ready
//	*/
//	while (SPI2STATbits.SPITBE == 0);
//
//	/* Write the next transmit unsigned char.
//	*/
//	SPI2BUF = bVal;
//
//	/* Wait for receive unsigned char.
//	*/
//	while (SPI2STATbits.SPIRBF == 0);
//
//	/* Put the received unsigned char in the buffer.
//	*/
//	bRx = SPI2BUF;
//
//	/* Bring the slave select line high
//	*/
//	PORTSetBits(prtSelect, bitSelect);
//	
//	return bRx;
//
//}

/* ------------------------------------------------------------ */
/***	ProcName
**
**	Parameters:
**
**	Return Value:
**
**	Errors:
**
**	Description:
**
*/

/* ------------------------------------------------------------ */

/************************************************************************/

