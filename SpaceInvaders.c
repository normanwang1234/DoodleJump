// SpaceInvaders.c
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 11/20/2018 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2018

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2018

 Copyright 2018 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// Start button connected to PE0
// Weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)


// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "Random.h"
#include "PLL.h"
#include "ADC.h"

#include "Images.h"
#include "Sound.h"
#include "Timer0.h"
#include "Timer1.h"
#include "PLL.h"

#define UPWARDS 0;
#define DOWNWARDS 1;
#define DEAD 2;
#define START 3;
#define PLAYING 4
#define NEXTMAP 5;
#define PAUSE 6;
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds

uint32_t Data;        // 12-bit ADC
uint32_t Position;    // 32-bit fixed-point 0.001 cm
int spriteX,spriteY;
int bulletX, bulletY;
int startBullet=0;
int spriteState=0;
int stateBounce=0;
int x,y,dropp;
int mail=0;
int status=0;
int gameState=3;
int score=0;
int germX;
int germY;
int largeGuyX;
int largeGuyY;
int count=0;
int numberPlatforms;
int xGerm[1];
int yGerm[1];
int xLarge[1];
int yLarge[1];


uint32_t padX[5];
uint32_t padY[5];
void PortF_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x30;      // 1) activate clock for Port F & E
	volatile int delay;
	delay++;
  GPIO_PORTF_DEN_R |= 0x4;
	GPIO_PORTF_DIR_R |= 0x4;
	GPIO_PORTF_DATA_R ^= 0x4;
	GPIO_PORTE_DIR_R &= ~0x3;		
	GPIO_PORTE_DEN_R |= 0x3; //PE0 is start/pause, PE1 is shooting
	NVIC_ST_CTRL_R=0;
	NVIC_ST_RELOAD_R=7256;
	NVIC_ST_CURRENT_R=0;
	NVIC_ST_CTRL_R|=0X07;
}
uint32_t Convert(uint32_t input){
  return (0.4941*input - 9.775)/160;
}

int main(void){
  PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
  Output_Init();
	ST7735_InitR(INITR_REDTAB); 
  PortF_Init();
  ADC_Init(); 
	Timer0_Init(7256);
  EnableInterrupts();			
//-**************************************************
 	ST7735_FillScreen(0xFFFF);  // set screen to white
  ST7735_SetCursor(5,10);
	ST7735_OutString("Press Button");
	ST7735_SetCursor(5,11);
	ST7735_OutString("To Start!");
	

  ST7735_DrawBitmap(52, 159, doodle, 23,19); //draws first doodle
	//52, 159
	ST7735_DrawBitmap(10,54, largeGuy, 40,22);
	ST7735_DrawBitmap(50,94, germ, 17,17);
	int x1=20;
	int y1=60;
  ST7735_DrawBitmap(x1,y1, GreenPad, 24,3);
	int x2=30;
	int y2=127;
	ST7735_DrawBitmap(x2,y2, GreenPad, 24,3);
	int x3=86;
	int y3=400;
	ST7735_DrawBitmap(x3,y3, GreenPad, 24,3);
	int x4=70;
	int y4=100;
	ST7735_DrawBitmap(x4,y4, BluePad, 24,5);

	while((GPIO_PORTE_DATA_R&0x1) == 0){};
	if((GPIO_PORTE_DATA_R&0x1)==1 && spriteState!=2)
		gameState=3;
	
  ST7735_FillScreen(0xFFFF);            // set screen to white
	


  while(1){
//********************************HERE START RANDOM MAP GENERATION***********************************************
	ST7735_FillScreen(0xFFFF);
	ST7735_SetCursor(0,0);
	ST7735_OutString("Score:");
	LCD_OutDec(score);
			int largeGuyX=Random32()%108;
			int largeGuyY=20+Random32()%20;
			int germX=10+Random32()%108;
			int germY=60+Random32()%20;
		
  if(gameState==3 || gameState==5)
	{
		numberPlatforms=2+Random32()%5;
		
	}
	if(numberPlatforms>=3 && (gameState==3 || gameState==5)) //3 is Start, 4 is Playing, 5 is nextMap
	{
		if(numberPlatforms==4)
		{

			xLarge[0]=largeGuyX;
			yLarge[0]=largeGuyY;
			ST7735_DrawBitmap(xLarge[0], yLarge[0], largeGuy, 40,22);

		}
		if(numberPlatforms==5 || numberPlatforms==6)
		{

			xGerm[0]=germX;
			yGerm[0]=germY;
			ST7735_DrawBitmap(xGerm[0], yGerm[0], germ, 17,17);			

		}
		int greenPadX=10+Random32()%108;
		int greenPadY=105+Random32()%10; 
				padY[0]=greenPadY;
				padX[0]=greenPadX;
				spriteX=padX[0];
				spriteY=padY[0]-20;
				
			
		int greenPadX1=10+Random32()%108;
		int greenPadY1=70+Random32()%10;
				padY[1]=greenPadY1;
				padX[1]=greenPadX1;
			
		int greenPadX2=10+Random32()%108;
		int greenPadY2=35+Random32()%10;
				padY[2]=greenPadY2;
				padX[2]=greenPadX2;
		
		int greenPadX3=10+Random32()%108;
		int greenPadY3=120+Random32()%20;
				padY[3]=greenPadY3;
				padX[3]=greenPadX3;

				gameState=4;
		
	}
				
		if(numberPlatforms>=3 && gameState==4)
		{
				ST7735_DrawBitmap(padX[0], padY[0], GreenPad, 24 ,3);
				ST7735_DrawBitmap(padX[1], padY[1], GreenPad, 24 ,3);
				ST7735_DrawBitmap(padX[2], padY[2], GreenPad, 24 ,3);
				ST7735_DrawBitmap(padX[3], padY[3], GreenPad, 24 ,3);
		}


	
	if(numberPlatforms<3 && (gameState==3 || gameState==5))
	{
		int greenPadX=10+Random32()%108;
		int greenPadY=125+Random32()%20; 
		padY[0]=greenPadY;
		padX[0]=greenPadX;
		
		spriteX=padX[0];
		spriteY=padY[0]-20;

		
		int greenPadX1=10+Random32()%108;
		int greenPadY1=100+Random32()%20;
		
		padY[1]=greenPadY1;
		padX[1]=greenPadX1;
		
		int greenPadX2=10+Random32()%108;
		int greenPadY2=75+Random32()%20; //75
		padY[2]=greenPadY2;
		padX[2]=greenPadX2;
	
		int greenPadX3=10+Random32()%108;
		int greenPadY3=50+Random32()%20;
		padY[3]=greenPadY3;
		padX[3]=greenPadX3;
		gameState=4;
		
		int greenPadX4=10+Random32()%108;
		int greenPadY4=25+Random32()%20;
		padY[4]=greenPadY4;
		padX[4]=greenPadX4;
	}
	
	if(numberPlatforms<3 && gameState==4)
	{
				ST7735_DrawBitmap(padX[0], padY[0], GreenPad, 24 ,3);
				ST7735_DrawBitmap(padX[1], padY[1], GreenPad, 24 ,3);
				ST7735_DrawBitmap(padX[2], padY[2], GreenPad, 24 ,3);
				ST7735_DrawBitmap(padX[3], padY[3], GreenPad, 24 ,3);
				ST7735_DrawBitmap(padX[4], padY[4], GreenPad, 24, 3);
	}

		

	
//****************************************HERE END RANDOM MAP GENERATION***********************************************8		
	while(status==0){};
				status=0;
				Data=mail;

	if(spriteState==2) //2 is DEAD
	{
		DisableInterrupts();
		ST7735_FillScreen(0xFFFF);
		ST7735_DrawBitmap(6,60, game, 44,18);
		ST7735_DrawBitmap(82,60, over, 40,15);
		ST7735_DrawBitmap(20,100, germ, 17,17);			//This is the final screen when doodle dies
		ST7735_DrawBitmap(86,130, fly, 18,15);

		ST7735_SetCursor(0,0);
		ST7735_OutString("Score:");
		LCD_OutDec(score);
		score=0;
		//ST7735_SetCursor(0,14);
		//ST7735_OutString("Press");
		//ST7735_SetCursor(0,15);
		//ST7735_OutString("Button To Start!");
		dropp=0;
		
					xGerm[0]=0XFFFF;
					yGerm[0]=0XFFFF;
					xLarge[0]=0XFFFF;
					yLarge[0]=0XFFFF;
		do
		{
			for(int dropp=0; dropp<200; dropp++)
			{
				ST7735_DrawBitmap(52, dropp, doodle, 23,19);
				if((GPIO_PORTE_DATA_R&0x01)==0x01)
				{
					spriteState=0;
					gameState=3;
					EnableInterrupts();
					break;
				}
				for(int x=0; x<100000;x++){}
			}
			
		}while(spriteState==2);

		
	}

				//LargeGuy W,H (40,22)
				//Germ W,H (17,17)
				
				
				if(spriteY>160) //If doodle falls to bottom of screen
				{
					spriteState=DEAD;
				}
				//checks to see if doodle approaches from the left
				if(((spriteX<=(xGerm[0]+8))&& (spriteX>=xGerm[0])&&(((spriteY+8)>=yGerm[0])&&(spriteY+8<=(yGerm[0]+16))))||((spriteX<=(xLarge[0]+39))&&(spriteX>=xLarge[0])&&(((spriteY+8)>=yLarge[0])&&((spriteY+8)<=(yLarge[0]+23)))))
							spriteState=DEAD;
				//checks to see if doodle approaches from the right
				if((((spriteX+10)>=xGerm[0])&& ((spriteX+10)<=xGerm[0]+20) &&(((spriteY+8)>=yGerm[0])&&(spriteY+8<=(yGerm[0]+16))))||(((spriteX+17)>=xLarge[0])&& ((spriteX+10)<=xLarge[0]+48)&&(((spriteY+8)>=yLarge[0])&&((spriteY+8)<=(yLarge[0]+23)))))
							spriteState=DEAD;
				if(spriteY<10) //If doodle reaches top of screen
				{
					spriteY=padY[0];
					GPIO_PORTF_DATA_R^=0X01;
					gameState=5; //5 is newMap
					ST7735_SetCursor(0,0);
					ST7735_OutString("Score: ");
					score+=Random32()%100;
					LCD_OutDec(score);
					xGerm[0]=0XFFFF;
					yGerm[0]=0XFFFF;
					xLarge[0]=0XFFFF;
					yLarge[0]=0XFFFF;
					spriteState=1; //Sprite goes downwards after newMap
					
				}



//********************************CODE TO GO UPWARDS***********************************************
				if(spriteState==0 && gameState==4) //If Sprite is going upwards & gameState=RUNNING
				{
					if((numberPlatforms>=3 && ((spriteX>=padX[0]-10 && spriteX+23<=padX[0]+34 && spriteY>=padY[0]-3 && spriteY<=padY[0]+5) || (spriteX>=padX[1]-10 && spriteX+23<=padX[1]+34 && spriteY>=padY[1]-3 && spriteY<=padY[1]+5) || (spriteX>=padX[2]-10 && spriteX+23<=padX[2]+34 && spriteY>=padY[2]-3 && spriteY<=padY[2]+5) || (spriteX>=padX[3]-10 && spriteX+23<=padX[3]+34 && spriteY>=padY[3]-3 && spriteY<=padY[3]+5))) || (numberPlatforms<3 && ((spriteX>=padX[0]-10 && spriteX+23<=padX[0]+34 && spriteY>=padY[0]-3 && spriteY<=padY[0]+5)  || (spriteX>=padX[1]-10 && spriteX+23<=padX[1]+34 && spriteY>=padY[1]-3 && spriteY<=padY[1]+5) || (spriteX>=padX[2]-10 && spriteX+23<=padX[2]+34 && spriteY>=padY[2]-3 && spriteY<=padY[2]+5) || (spriteX>=padX[3]-10 && spriteX+23<=padX[3]+34 && spriteY>=padY[3]-3 && spriteY<=padY[3]+5) || (spriteX>=padX[4]-10 && spriteX+23<=padX[4]+34 && spriteY>=padY[4]-3 && spriteY<=padY[4]+5))))
					{
					while(1)
					{ //Goes upwards
						ST7735_DrawBitmap(spriteX, spriteY, doodle, 23 ,19);
						if(numberPlatforms==4)
						{
							xLarge[0]-=1;
							ST7735_DrawBitmap(xLarge[0], yLarge[0], largeGuy, 40,22);
						}
						if(numberPlatforms==5 || numberPlatforms==6)
						{
							xGerm[0]+=1;
							ST7735_DrawBitmap(xGerm[0], yGerm[0], germ, 17,17);
						}
						if(numberPlatforms>=3)
						{
							ST7735_DrawBitmap(padX[0], padY[0], GreenPad, 24 ,3);
							ST7735_DrawBitmap(padX[1], padY[1], GreenPad, 24 ,3);
							ST7735_DrawBitmap(padX[2], padY[2], GreenPad, 24 ,3);
							ST7735_DrawBitmap(padX[3], padY[3], GreenPad, 24 ,3);
						}
						if(numberPlatforms<3)
						{
							ST7735_DrawBitmap(padX[0], padY[0], GreenPad, 24 ,3);
							ST7735_DrawBitmap(padX[1], padY[1], GreenPad, 24 ,3);
							ST7735_DrawBitmap(padX[2], padY[2], GreenPad, 24 ,3);
							ST7735_DrawBitmap(padX[3], padY[3], GreenPad, 24, 3);
							ST7735_DrawBitmap(padX[4], padY[4], GreenPad, 24, 3);
						}
						count++;
				
						if(count<26)
							spriteY-=2;
						else
							spriteY-=1;
						if(count==30)
						{
							count=0;
							spriteState=DOWNWARDS;
							break;
						}
					
							for(x=100000;x>0;x--){
							};
					}
				}
			}
		
				if(spriteState==1 && gameState==4)
				{
					if((spriteX+23<=xLarge[0]+56 && spriteX>=xLarge[0]-8)  && (spriteY>=yLarge[0]+44 && spriteY<=yLarge[0]+50)) //Checks if sprite bounce on head of Large guy
					{
						spriteState=UPWARDS;
						ST7735_FillRect(xLarge[0], yLarge[0], 40, 22, ST7735_WHITE); //Erases large guy if bounce on top of head
						xLarge[0]=0XFFFF;
						yLarge[0]=0XFFFF;
						ST7735_FillScreen(0XFFFF);
					}
					if((spriteX+23<=xGerm[0]+24 && spriteX>=xGerm[0]-7)  && (spriteY>=yGerm[0]+19 && spriteY<=yGerm[0]+25)) //Checks if sprite bounce on head of Germ
					{
						spriteState=UPWARDS;
						ST7735_FillRect(xGerm[0], yGerm[0], 17, 17, ST7735_WHITE); //Erases germ if bounce on top of head
						xGerm[0]=0XFFFF;
						yGerm[0]=0XFFFF;
						ST7735_FillScreen(0XFFFF);
					}
					if((numberPlatforms>=3 && ((spriteX>=padX[0]-10 && spriteX+23<=padX[0]+34 && spriteY>=padY[0]-3 && spriteY<=padY[0]+5) || (spriteX>=padX[1]-10 && spriteX+23<=padX[1]+34 && spriteY>=padY[1]-3 && spriteY<=padY[1]+5) || (spriteX>=padX[2]-10 && spriteX+23<=padX[2]+34 && spriteY>=padY[2]-3 && spriteY<=padY[2]+5) || (spriteX>=padX[3]-10 && spriteX+23<=padX[3]+34 && spriteY>=padY[3]-3 && spriteY<=padY[3]+5))) || (numberPlatforms<3 && ((spriteX>=padX[0]-10 && spriteX+23<=padX[0]+34 && spriteY>=padY[0]-3 && spriteY<=padY[0]+5)  || (spriteX>=padX[1]-10 && spriteX+23<=padX[1]+34 && spriteY>=padY[1]-3 && spriteY<=padY[1]+5) || (spriteX>=padX[2]-10 && spriteX+23<=padX[2]+34 && spriteY>=padY[2]-3 && spriteY<=padY[2]+5) || (spriteX>=padX[3]-10 && spriteX+23<=padX[3]+34 && spriteY>=padY[3]-3 && spriteY<=padY[3]+5) || (spriteX>=padX[4]-10 && spriteX+23<=padX[4]+34 && spriteY>=padY[4]-3 && spriteY<=padY[4]+5))))
					{
						//Checks if Sprite jumps on GreenPad
						spriteState=UPWARDS;
					}
				}
				if(gameState==4 && spriteState!=2)
					spriteY+=3;
				//Sprite falling
				ST7735_DrawBitmap(spriteX, spriteY, doodle, 23 ,19);
				
						if(numberPlatforms>=3)
						{
							ST7735_DrawBitmap(padX[0], padY[0], GreenPad, 24 ,3);
							ST7735_DrawBitmap(padX[1], padY[1], GreenPad, 24 ,3);
							ST7735_DrawBitmap(padX[2], padY[2], GreenPad, 24 ,3);
							ST7735_DrawBitmap(padX[3], padY[3], GreenPad, 24 ,3);
						}
						if(numberPlatforms<3)
						{
							ST7735_DrawBitmap(padX[0], padY[0], GreenPad, 24 ,3);
							ST7735_DrawBitmap(padX[1], padY[1], GreenPad, 24 ,3);
							ST7735_DrawBitmap(padX[2], padY[2], GreenPad, 24 ,3);
							ST7735_DrawBitmap(padX[3], padY[3], GreenPad, 24, 3);
							ST7735_DrawBitmap(padX[4], padY[4], GreenPad, 24, 3);
						}
						if((numberPlatforms==5 || numberPlatforms==6)&& xGerm[0]<10)
						{
							ST7735_DrawBitmap(xGerm[0], yGerm[0], germ, 17,17);			//Draws and makes germ zone
							xGerm[0]+=1; 
						}
						else if((numberPlatforms==5 || numberPlatforms==6)&& (xGerm[0]+17)>124)
						{
							ST7735_DrawBitmap(xGerm[0], yGerm[0], germ, 17,17);			//Draws and makes germ zone
							xGerm[0]-=1;
						}
						else if(numberPlatforms==5 || numberPlatforms==6)
						{
							ST7735_DrawBitmap(xGerm[0], yGerm[0], germ, 17, 17); 		//Draws and Moves germ
							xGerm[0]-=1;
						}
						if(numberPlatforms==4 && xLarge[0]<10)
						{
							ST7735_DrawBitmap(xLarge[0], yLarge[0], largeGuy, 40,22);			//Draws and makes large guy zone
							xLarge[0]+=1;
						}
						else if(numberPlatforms==4 && (xLarge[0]+44)>124)
						{
							ST7735_DrawBitmap(xLarge[0], yLarge[0], largeGuy, 40,22);			//Draws and makes large guy zone
							xLarge[0]-=1;
						}
						else if(numberPlatforms==4)
						{
							ST7735_DrawBitmap(xLarge[0], yLarge[0], largeGuy, 40, 22); //Draws and moves large guy
							xLarge[0]+=1;
						}

						

					for(x=50000;x>0;x--){}; //Delay
				
			}

					

					
				
				
		}
		
			
							
			

//**************************************************


// You can't use this timer, it is here for starter code only 
// you must use interrupts to perform delays
void Delay100ms(uint32_t count)
{
	uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}
int counter=0;
void SysTick_Handler(void){
	status=1;
	mail=ADC_In();
	GPIO_PORTF_DATA_R ^= 0x04;


				//******Shooting Implementation, PE1******
				if((GPIO_PORTE_DATA_R&0x02)>>1==0x01) //Checks if PORTE1 is pressed
				{
					startBullet=1; //Sets true
					bulletX=spriteX+11; //Bullet comes out of body of Sprite
					bulletY=spriteY;
				}
				if(startBullet==1)
				{
					ST7735_FillRect(bulletX, bulletY+2, 3, 3 , ST7735_WHITE);
					bulletY-=2; //Moves bullet straight upward
					ST7735_FillRect(bulletX, bulletY, 3, 3, ST7735_RED);
				}
				if(bulletY<0)
				{
					startBullet=0;
					ST7735_FillRect(bulletX, bulletY, 3, 3, ST7735_WHITE);
					bulletX=0xFFFF;
					bulletY=0XFFFF;
				}
				if(bulletX<xLarge[0]+44 && bulletX>xLarge[0] && bulletY<=yLarge[0]+12) //Checks if bullet hits large guy
				{
					ST7735_FillRect(xLarge[0], yLarge[0], 44, 22, ST7735_WHITE);
					xLarge[0]=0xFFFF;
					yLarge[0]=0XFFFF;
				}
				if(bulletX<xGerm[0]+17 && bulletX>xGerm[0] && bulletY<yGerm[0]+8) //Checks if bullet hits germ
				{
					ST7735_FillRect(xGerm[0], yGerm[0], 17,17, ST7735_WHITE);
					xGerm[0]=0XFFFF;
					yGerm[0]=0XFFFF;
				}
				
				//*********PAUSE*************
				if((GPIO_PORTE_DATA_R&0x01)==0x01 && gameState==4) //Checks PORTE0 (4 is Running)
				{
					gameState=PAUSE; //Pauses Game
				}
				else
						spriteX=mail*0.027;
				while(gameState==6 && (GPIO_PORTE_DATA_R&0X01)==0X01){}; 
				if((GPIO_PORTE_DATA_R&0x01)==0x00 && gameState==6) //Checks if PORTE0 is released and unpauses on release(6 is Pause)
				{
					gameState=PLAYING;
				}
}
