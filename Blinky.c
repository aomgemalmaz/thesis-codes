/*----------------------------------------------------------------------------
 *      
 *----------------------------------------------------------------------------
 *      Name:    BLINKY.C
 *      Purpose: Bare Metal example program
 *----------------------------------------------------------------------------
 *      
 *      Copyright (c) 2006-2014 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include "MKL25Z4.h"                    // Device header
#include "TIspi.h"											//spi.h


#define 	WRITE_BURST     		0x40						//write burst
#define 	READ_SINGLE     		0x80						//read single
#define 	READ_BURST      		0xC0						//read burst
#define 	BYTES_IN_RXFIFO     0x7F  					//byte number in RXfifo



/**********************************************************************
***********							 Macros       		  	*************************
***********************************************************************	
	Aim  : Macros are defined here
	NOTE : 
**********************************************************************/

							/*LEDS*/
#define RED_OFF 		(FPTB->PSOR |= (1<<18))
#define RED_ON  		(FPTB->PCOR |= (1<<18),FPTB->PSOR |= (1<<19),(FPTD->PSOR  = 0x01))
#define GREEN_OFF 	(FPTB->PSOR |= (1<<19))
#define GREEN_ON  	(FPTB->PSOR |= (1<<18),FPTB->PCOR |= (1<<19),(FPTD->PSOR  = 0x01))
#define YELLOW_OFF 	(FPTB->PSOR |= (1<<18),FPTB->PSOR |= (1<<19),(FPTD->PSOR  = 0x01))
#define YELLOW_ON  	(FPTB->PCOR |= (1<<18),FPTB->PCOR |= (1<<19),(FPTD->PSOR  = 0x01))

#define SpiStart()			FPTD->PCOR |= (1UL<<4);                             	// CS=low,  SPI start
#define SpiStop()				FPTD->PSOR |= (1UL<<4);                             	// CS=high, SPI stop

//#define SpiStart()			PTA->PCOR |= (1UL);                             	// CS=low,  SPI start
//#define SpiStop()				PTA->PSOR |= (1UL);                             	// CS=high, SPI stop

#define wait_CHIP_RDYn	while((FPTD->PDIR & 0x80)!= 0)											//Wait for CHIP_RDYn signal
//#define wait_CHIP_RDYn	while((SPI_Send(0x00) & 0x80) != 0);

/**********************************************************************
***********							 Global Values		  	*************************
***********************************************************************	
	Aim  : Global registers are defined here
	NOTE : 
**********************************************************************/
char test[11];
uint32_t hede;
/**********************************************************************
***********							 Systick					  	*************************
***********************************************************************	
	Aim  : initialize Systick Timer
	NOTE : 1 ms Ticks
**********************************************************************/
volatile uint32_t msTicks;                            /* counts 1ms timeTicks */
void SysTick_Handler(void) {
  msTicks++;                        /* increment counter necessary in Delay() */
}
__INLINE static void Delay (uint32_t dlyTicks) {
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks);
}
/**********************************************************************
***********							 LED_Init							*************************
***********************************************************************	
	Aim  : initialize the leds
	NOTE : 18>Red---19>Green---- 18+19>Yellow
**********************************************************************/
void LED_Init(void) {

   
  PORTB->PCR[18] = PORT_PCR_MUX(1);                     /* Pin PTB18 is GPIO-RED */ 
  PORTB->PCR[19] = PORT_PCR_MUX(1);                     /* Pin PTB19 is GPIO-GREEN */
	PORTD->PCR[1]  = PORT_PCR_MUX(1);                     /* Pin PTD1 is GPIO-BLUE */
	FPTB->PDDR     |= (1UL<<18) | (1UL<<19);					 /* enable PTB18/19 as Output */
	FPTD->PDDR     |= (1UL);														// enable PTD1 as output
	//Turn off leds
	YELLOW_OFF;	GREEN_OFF;	RED_OFF;
	// Turn on leds 1 by 1 
	YELLOW_ON; Delay(1000);	GREEN_ON; Delay(1000);	RED_ON; Delay(1000);
	//Turn off leds
	YELLOW_OFF;	GREEN_OFF;	RED_OFF;
	
}
/**********************************************************************
***********							 SPI_Init							*************************
***********************************************************************	
	Aim  : initialize SPI for Kl25
	NOTE : 
**********************************************************************/
	void spi_init(void){
		  	 																								
		int dly;
		
    SIM->SCGC4 |= SIM_SCGC4_SPI1_MASK;        	 								//Enable SPI1 clock  
		
		
		//manual ss pin 
		PORTD->PCR[4] |= PORT_PCR_MUX(1); 													//SS pin gpio
		PTD->PDDR     |= (1UL<<4);																	//SS pin output PD4
//		PORTA->PCR[1]   |= PORT_PCR_MUX(1); 
//		PTA->PDDR   	  |= (1UL);																	
		
		PORTD->PCR[2] |= PORT_PCR_MUX(1); 													//TI_Reset pin gpio
		PTD->PDDR     |= (1UL<<2);																	//TI_Reset pin output PD4
		
		//PORTD->PCR[0] = PORT_PCR_MUX(0x2);           							//Set PTD4 to mux 2   (SS)
		PORTD->PCR[5] = PORT_PCR_MUX(0x02);           							//Set PTD5 to mux 2   (clk)
		PORTD->PCR[6] = PORT_PCR_MUX(0x02);           							//Set PTD6 to mux 2   (Mosi)
		PORTD->PCR[7] = PORT_PCR_MUX(0x02);           							//Set PTD7 to mux 2   (Miso)
		
		SPI1->C1 = SPI_C1_MSTR_MASK;         											//Set SPI0 to Master   
		SPI1->C2 = SPI_C2_MODFEN_MASK;                           	//Master SS pin acts as slave select output        
//		SPI1->BR = (SPI_BR_SPPR(0x111) | SPI_BR_SPR(0x1000));     		//Set baud rate prescale divisor to 3 & set baud rate divisor to 32 for baud rate of 15625 hz        
//		SPI1->BR |= 0x30;
			SPI1->BR |= 0x43;
		
//		SPI1->C1 |=  (1UL << 3) ; 										//SPI MOD 3
//		SPI1->C1 |=  (1UL << 2) ; 
		
		dly=1000;
		while(dly--);
		SPI1->C1 |= 0x40;																					//SPI1 Enable
		PTD->PSOR |= (1UL<<4);
		PTD->PSOR |= (1UL<<2);
		

	}
/**********************************************************************
***********							 SPI_SEND							*************************
***********************************************************************	
	Aim  :  send and recieve data via SPI
	NOTE : 
**********************************************************************/
char SPI_Send(char Data)
{
	while(!(SPI_S_SPTEF_MASK & SPI1->S));   
	SPI1->D = Data;																								//Write Data
	while(!(SPI_S_SPRF_MASK & SPI1->S)); 
	return 	SPI1->D;																							//Read Data

}
char testt;
char x;
/**********************************************************************
***********							 TI_HW_Reset							*************************
***********************************************************************	
	Aim  : Hardware Reset using MCU
	NOTE :active low for 200 ns, other times high
**********************************************************************/
void TI_HW_Reset(void)
{
	PTD->PCOR |= (1UL<<2);
	Delay(0x5);
	PTD->PSOR |= (1UL<<2);
}

/**********************************************************************
***********							 TI_Init							*************************
***********************************************************************	
	Aim  : wait for initialization of CC1120
	NOTE :
**********************************************************************/
void TI_Init(void)
{
	TI_HW_Reset();
	
	SpiStart();																										//Start SPI by CSn Low
	wait_CHIP_RDYn; 																							//Wait for TI's christal to be stabilized	
//	while((SPI_Send(0x00) & 0x80) != 0);
//	x= SPI_Send(0x00);
	//	Delay(0x5000);
	SpiStop();																										//Stop SPI by CSn High
	
}


/**********************************************************************
***********							 TI_Write							*************************
***********************************************************************	
	Aim  : write 1 byte data to chip via SPI
	NOTE :[ R/W bit (0)] + [ Burst bit (0)] + [6 bit addres] + 8 bit data
**********************************************************************/
void TI_WriteByte( char addr,  char data)
{
//int dly;
   SpiStart();																										//Start SPI by CSn Low
	 wait_CHIP_RDYn; 																								//Wait for TI's christal to be stabilized
	 SPI_Send(addr);																							  // R/w bit (1) + Burst bit (0)+ 6 bit addres
	 SPI_Send(data);
	 SpiStop();	                               											// CS=1,  SPI end
}

/**********************************************************************
***********							 TI_Read							*************************
***********************************************************************	
	Aim  : Read 1 byte data from chip via SPI
	NOTE :[ R/W bit (1)] + [ Burst bit (0)] + [6 bit addres]
**********************************************************************/
char TI_ReadByte(char addr)
{
   char data = 0;
	
	 SpiStart();																										//Start SPI by CSn Low
	 wait_CHIP_RDYn; 																								//Wait for TI's christal to be stabilized
	 SPI_Send( READ_SINGLE | addr);																  // R/w bit (1) + Burst bit (0)+ 6 bit addres
	 data = SPI_Send(0x00);             														// Data read (read 1byte data) via dummy write
	 SpiStop();																											//Stop SPI by CSn High
	
   return data;    
}
/*******************************************************************************/
/****************************** CC1120 WRITE  BUFFER **********************/
/*******************************************************************************/
int TI_Write_buf(unsigned int addrbsb,char* buf,int len)
{
   int i = 0;
	
   
		PTD->PCOR |= (1UL<<4);                             	  // CS=0, SPI start
		SPI_Send (  (addrbsb & 0x00FF0000)>>16);							// Address byte 1
		SPI_Send (  (addrbsb & 0x0000FF00)>> 8);	 						// Address byte 2
		SPI_Send (  (addrbsb & 0x000000F8) + 4);  						// Data write command and Write data length 1
	

	for(i = 0; i < len; i++)              						  // Write data in loop
   {
    SPI_Send (buf[i]);
   }
	 
   PTD->PSOR |= (1UL<<4);                                // CS=1, SPI end
    

   return len;  
}

/*******************************************************************************/
/****************************** CC1120 READ  BUFFER ***********************/
/*******************************************************************************/
int TI_Read_buf(unsigned int addrbsb, char* buf,int len)
{
  int i = 0;
 
	
  
  PTD->PCOR |=(1UL<<4);                                 // CS=0, SPI start
	
  SPI_Send ( (addrbsb & 0x00FF0000)>>16);   					// Address byte 1
  SPI_Send ( (addrbsb & 0x0000FF00)>> 8);   					// Address byte 2
  SPI_Send ( (addrbsb & 0x000000F8));       					// Data write command and Write data length 1
  
	for(i = 0; i < len; i++)                    			  // Write data in loop
  {
    buf[i] = SPI_Send(0x00);
  }
  PTD->PSOR |= (1UL<<4);                                  // CS=1, SPI end
                                                                                        
  return len;
}
char DelayUs(long t)																
{
do
	{
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
//	__NOP();
//	__NOP();
//	__NOP();
	} while(--t);
	return 1;
}
/**********************************************************************
***********							 Main Program						***********************
***********************************************************************	
	Aim  : main program
	NOTE :
**********************************************************************/
int main (void) 
	{
	
  SystemCoreClockUpdate();
//	SysTick_Config(10000);																					//turn SysTick timer on
	SysTick_Config(SystemCoreClock/1600);															// 1ms SysTicks
	Delay(100);																												//wait for system stabilization
	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTB_MASK;      	//Port-D-B clock ON 
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
	Delay(0x500);                     																//delay  
	LED_Init();           																						//Initialize the LEDs          
	spi_init();																												// SPI-1 init 
	Delay(0x100);																											//delay
	hede=PTD->PDIR;
	TI_Init();
//		TI_HW_Reset();
//	Delay(0x3000);													//delay for logic analyzer
	hede=PTD->PDIR;
//	SpiStart();	
//	SPI_Send(0x10);
//	SpiStop();
		test[0]=TI_ReadByte(0x0F);
		test[1]=TI_ReadByte(0x0F);
		test[2]=TI_ReadByte(0x0F);
		test[3]=TI_ReadByte(0x0F);
		test[4]=TI_ReadByte(0x0F);
		
	TI_WriteByte(CC112X_IOCFG3,0x87);
	test[5]=TI_ReadByte(CC112X_IOCFG3);
//	Delay(0x2000);
	while(1)
	{
//		SpiStart();
//		hede=PTD->PDIR;
//		DelayUs(0x1000);
//		SPI_Send(0x10);
//		x=SPI_Send(0x00);
//		x=SPI_Send(0x00);
//		SpiStop();
//		Delay(0x50);
		
//		SpiStart();
//		SPI_Send(0x10);
//		x=SPI_Send(0x00);
//		SpiStop();
//		hede=PTD->PDIR;
//		
//		SpiStop();
////		YELLOW_ON; Delay(1000);	GREEN_ON; Delay(1000);	RED_ON; Delay(1000);
////		TI_WriteByte(CC112X_IOCFG3,0x87);
////		test[2]=TI_ReadByte(CC112X_IOCFG3);
////		YELLOW_OFF;	GREEN_OFF;	RED_OFF;
//		
//	PTD->PCOR |= (1UL<<4);                             	  // CS=0, SPI start
//	
//	hede=PTD->PDIR;

////		test[0]=SPI_Send(0x10);
//	while((PTD->PDIR & 0x80)!= 0);												//Wait for CHIP_RDYn signal
//	
//	SPI_Send(0x8F);																//send the adress and get the status byte
//	test[2]= SPI_Send(0x00);															//read the adress
////		test[1]=SPI_Send(0x00);
////		test[2]=SPI_Send(0x10);
////		test[3]=SPI_Send(0x00);
////		SPI_Send(0x10);
//	
//	PTD->PSOR |= (1UL<<4);                             	  // CS=0, SPI stop
	}
}
