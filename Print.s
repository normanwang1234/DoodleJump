; Print.s
; Student names: Ethan Blumenfeld & Norman Wang
; Last modification date: change this to the last modification date or look very silly
; Runs on LM4F120 or TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

Remainder EQU 0x00
  

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutDec
	 PUSH {R0, LR} 
	 MOV R3, SP ;R3 has stack pointer
recursion	 
	 CMP R0, #10  
	 BLO branchOut
	 SUB SP, #8
	 MOV R2, #10 
	 UDIV R1, R0, R2   
	 MUL R2, R1, R2  
	 SUB R2, R0, R2  ;R2 had the remainder
	 MOV R0, R2  
	 STR R0, [SP, #Remainder] ;Store as local variable
	 MOV R0, R1  ;R0 has one less place, 723 turns into 72
	 B recursion 
	
branchOut
	 ADD R0, #0X30
	 PUSH{R0,R3}
	 BL ST7735_OutChar
	 POP{R0,R3}
	 
loop
	 CMP R3, SP
	 BEQ done
     LDR R0, [SP, #Remainder] ;loads local variables
	 ADD R0, #0X30
	 PUSH{R0,R3}
	 BL ST7735_OutChar 
	 POP{R0,R3}
	 ADD SP, #8 ;Goes down list and prints
	 B loop
done 
	 POP {R0, LR}
	 BX LR
	 


	 
	 
	 
	 
	 
	 
	 
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.000 "
;       R0=3,    then output "0.003 "
;       R0=89,   then output "0.089 "
;       R0=123,  then output "0.123 "
;       R0=9999, then output "9.999 "
;       R0>9999, then output "*.*** "
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix
 PUSH {R0, LR}
	 MOV R2, #1000
	 UDIV R1, R0, R2
	 CMP R1, #10
	 BHS star ;If it's greater than 10000, it prints *.***
	 MOV R3, R0 ;R3 temporarily has R0
	 
	 ADD R0, R1, #0x30 ;Prints ones place
	 PUSH {R0,R1,R2, R3}
	 BL ST7735_OutChar
	 MOV R0, #0x2E ;Prints .
	 BL ST7735_OutChar
	 POP {R0,R1,R2, R3}
	 MOV R0, R3 ;R0 has original value
	 MUL R2, R1, R2 ;R2 has remainder
	 SUB R2, R0, R2
	 CMP R2, #10 ;Compares # to see if less than 10, then it prints 2 zeros
	 BLO twoZeros
	 CMP R2, #100
	 BLO oneZero ;Compares # to see if less than 100, then it prints 1 zero
	 MOV R0, R2
	 BL LCD_OutDec 
	 POP {R0, LR}
	 BX LR

	 
	 
	 
oneZero
	 MOV R0, #0x30 ;R0 has zero ascii
	 PUSH {R0, R2}
	 BL ST7735_OutChar
	 POP {R0, R2}
	 MOV R0, R2
	 BL LCD_OutDec ;prints rest
	 POP {R0, LR}
	 BX LR
	 
twoZeros
	 MOV R0, #0X30 ;R0 has zero ascii
	 PUSH {R0, R2}
	 BL ST7735_OutChar
	 MOV R0, #0X30 ;Prints zero twice
	 BL ST7735_OutChar
	 POP {R0, R2}
	 MOV R0, R2
	 BL LCD_OutDec
	 POP {R0, LR}
	 BX LR
	 
star
	 MOV R0, #0X2A ;Print *
	 BL ST7735_OutChar
	 MOV R0, #0X2E ;Print .
	 BL ST7735_OutChar
	 MOV R0, #0X2A ;Print *
	 BL ST7735_OutChar
	 MOV R0, #0X2A ;Print *
	 BL ST7735_OutChar
	 MOV R0, #0X2A ;Print *
	 BL ST7735_OutChar
	 POP {R0, LR}
	 BX LR
 
    
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN                           ; make sure the end of this section is aligned
     END                             ; end of file
