StellarisPmodOLED
=================

Porting the Digilent driver code for the Digilent Pmod OLED to run on Stellaris Launchpad.

This code is ported from example code provided for Digilent for the Pmod OLED display to 
run on the PIC32 Cerebot MX4cK (this is now the chipKIT™ Pro MX4).
http://www.digilentinc.com/Products/Detail.cfm?NavPath=2,401,963&Prod=PMOD-OLED
http://www.digilentinc.com/Products/Detail.cfm?NavPath=2,892,1218&Prod=CHIPKIT-PRO-MX4

It is currently set up to use SSI3 for SPI control, and 4 pins from Port B for other control signals.  
(These are close on the board).  Changing the data/cmd pin from PB4 to PE4 could make it easier to use 
a 2x6 connector to connect the PMOD to the Launchpad (with wires swapped around between connectors, 
would need to ensure the correct side was connected to each board in the correct orientation).

Note, that Digilent sells a Stellaris Booster pack that includes an OLED display and more for just 
2 dollars more.  It also includes additional peripherals and two Pmod ports.


