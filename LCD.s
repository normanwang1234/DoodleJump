; LCD.s
; Student names: Ethan Blumenfeld & Norman Wang
; Last modification date: 4/1/19

; Runs on LM4F120/TM4C123
; Use SSI0 to send an 8-bit code to the ST7735 160x128 pixel LCD.

; As part of Lab 7, students need to implement these LCD_WriteCommand and LCD_WriteData
; This driver assumes two low-level LCD functions

; Backlight (pin 10) connected to +3.3 V
; MISO (pin 9) unconnected
; SCK (pin 8) connected to PA2 (SSI0Clk)
; MOSI (pin 7) connected to PA5 (SSI0Tx)
; TFT_CS (pin 6) connected to PA3 (SSI0Fss)
; CARD_CS (pin 5) unconnected
; Data/Command (pin 4) connected to PA6 (GPIO)
; RESET (pin 3) connected to PA7 (GPIO)
; VCC (pin 2) connected to +3.3 V
; Gnd (pin 1) connected to ground

DC                      EQU   0x40004100
DC_COMMAND              EQU   0
DC_DATA                 EQU   0x40
SSI0_DR_R               EQU   0x40008008
SSI0_SR_R               EQU   0x4000800C
SSI_SR_RNE              EQU   0x00000004  ; SSI Receive FIFO Not Empty
SSI_SR_BSY              EQU   0x00000010  ; SSI Busy Bit
SSI_SR_TNF              EQU   0x00000002  ; SSI Transmit FIFO Not Full
PA_DATA 				EQU	  0x400043FC


      EXPORT   writecommand
      EXPORT   writedata

      AREA    |.text|, CODE, READONLY, ALIGN=2
      THUMB
      ALIGN

; The Data/Command pin must be valid when the eighth bit is
; sent.  The SSI module has hardware input and output FIFOs
; that are 8 locations deep.  Based on the observation that
; the LCD interface tends to send a few commands and then a
; lot of data, the FIFOs are not used when writing
; commands, and they are used when writing data.  This
; ensures that the Data/Command pin status matches the byte
; that is actually being transmitted.
; The write command operation waits until all data has been
; sent, configures the Data/Command pin for commands, sends
; the command, and then waits for the transmission to
; finish.
; The write data operation waits until there is room in the
; transmit FIFO, configures the Data/Command pin for data,
; and then adds the data to the transmit FIFO.
; NOTE: These functions will crash or stall indefinitely if
; the SSI0 module is not initialized and enabled.

; This is a helper function that sends an 8-bit command to the LCD.
; Input: R0  8-bit command to transmit
; Output: none
; Assumes: SSI0 and port A have already been initialized and enabled
writecommand
;; --UUU-- Code to write a command to the LCD
;1) Read SSI0_SR_R and check bit 4, 
;2) If bit 4 is high, loop back to step 1 (wait for BUSY bit to be low)
;3) Clear D/C=PA6 to zero
;4) Write the command to SSI0_DR_R
;5) Read SSI0_SR_R and check bit 4, 
;6) If bit 4 is high, loop back to step 5 (wait for BUSY bit to be low)
LOOP
	LDR R1,=SSI0_SR_R
	LDR R1,[R1]
	AND R1, #0X10			;1
	SUBS R1,#0x10
	BEQ LOOP				;2
	LDR R2,=DC
	LDR R3, [R2]
	BIC R3, #0x40
	STR R3, [R2] 			;3
	
	LDR R1, =SSI0_DR_R 		
	STR R0, [R1]			;4
	 		
	
LOOP2
	LDR R2, =SSI0_SR_R
	LDR R2, [R2]
	AND R2, #0X10
	SUBS R2, #0X10 			;5
	BEQ LOOP2 				;6
	
	

    BX  LR                          ;   return

; This is a helper function that sends an 8-bit data to the LCD.
; Input: R0  8-bit data to transmit
; Output: none
; Assumes: SSI0 and port A have already been initialized and enabled
writedata
;; --UUU-- Code to write data to the LCD
;1) Read SSI0_SR_R and check bit 1, 
;2) If bit 1 is low loop back to step 1 (wait for TNF bit to be high)
;3) Set D/C=PA6 to one
;4) Write the 8-bit data to SSI0_DR_R
LOOP3

	LDR R1, =SSI0_SR_R
	LDRB R1,[R1]
    AND R1,#0x01			;1
	CMP R1,#0x00
	BEQ LOOP3				;2
	LDR R1,=DC
	LDRB R2,[R1]
	ORR R2,#0x40
	STRB R2,[R1]			;3
	LDR R1,=SSI0_DR_R
	STRB R0,[R1]			;4
    
    BX  LR                          ;   return

	ALIGN
	END