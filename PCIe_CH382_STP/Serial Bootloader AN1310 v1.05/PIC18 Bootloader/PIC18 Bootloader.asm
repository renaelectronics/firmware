; Copyright (c) 2002-2011,  Microchip Technology Inc.
;
; Microchip licenses this software to you solely for use with Microchip
; products.  The software is owned by Microchip and its licensors, and
; is protected under applicable copyright laws.  All rights reserved.
;
; SOFTWARE IS PROVIDED "AS IS."  MICROCHIP EXPRESSLY DISCLAIMS ANY
; WARRANTY OF ANY KIND, WHETHER EXPRESS OR IMPLIED, INCLUDING BUT
; NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
; FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.  IN NO EVENT SHALL
; MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
; CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, HARM TO YOUR
; EQUIPMENT, COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY
; OR SERVICES, ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED
; TO ANY DEFENSE THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION,
; OR OTHER SIMILAR COSTS.
;
; To the fullest extent allowed by law, Microchip and its licensors
; liability shall not exceed the amount of fees, if any, that you
; have paid directly to Microchip to use this software.
;
; MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE
; OF THESE TERMS.
;
; Author        Date        Comment
; ************************************************************************
; E. Schlunder  07/20/2010  Software Boot Block Write Protect code 
;                           improved. 96KB memory size devices should
;                           work now.
; E. Schlunder  02/26/2010  Changed order of start up code so that PLLEN
;                           is enabled before we wait for RXD IDLE state. 
;                           This improves connection time/reliablity on 
;                           J devices.
; E. Schlunder  08/28/2009  Software Boot Block Write Protect option.
; E. Schlunder  07/09/2009  Brought back support for bootloader at 
;                           address 0 for hardware boot block write 
;                           protection on certain devices.
; E. Schlunder  05/07/2009  Replaced the simple checksum with a
;                           16-bit CCIT CRC checksum. 
;                           Added ReadFlashCrc command for quick verify.
; E. Schlunder  05/02/2009  Improved autobaud code to handle 1Mbps
;                           and BRG16/BRGH.
; E. Schlunder  05/01/2009  Added support for DEVICES.INC generated
;                           from Device Database tool. 
; E. Schlunder  04/29/2009  Added support for locating the bootloader
;                           at the end of program memory instead of
;                           the beginning. This will eventually let us
;                           use normal application firmware code without
;                           linker script modifications.
; E. Schlunder  04/26/2009  Optimized Config Write routine to avoid 
;                           re-writing values matching existing config
;                           data.
; E. Schlunder  04/24/2009  Optimized EEPROM Write routine a little bit.
;                           Optimized FLASH Read routine to stream data
;                           directly from FLASH instead of using RAM
;                           buffer.
;                           Added option for faster STX acknowledgements.
; E. Schlunder  04/17/2009  Added code to enter bootloader mode if
;                           a serial break condition is detected on
;                           RXD as we come out of reset. This will
;                           make it possible to re-enter the bootloader
;                           even if the application firmware is missing
;                           code to re-enter bootloader mode.
;                           This also simplies the bootloader, as
;                           we do not need a boot flag any more.
; E. Schlunder  04/15/2009  Removed EOF command 8, new PC software
;                           does 64 byte block aligned writes at all
;                           times on J device, so there is no need for
;                           this command going forward.
; E. Schlunder  04/14/2009  Added a BootloadMode vector back at the 
;                           beginning of program memory so that user
;                           applications can jump back into the boot 
;                           loader without having to erase the boot flag.
; E. Schlunder  04/08/2009  Now initializes FSR2 to 0 so that the code 
;                           can operate under Extended Instruction Set
;                           mode if necessary.
; E. Schlunder  04/01/2009  Fixed bug in J_FLASH erase address increment.
;                           Added support for enabling PLL.
;                           Added support for inverted UART signaling.
;                           Added support for fixed (non-autobaud) 
;                           operation, helps with debugging code under ICD.
; E. Schlunder  03/25/2009  No longer attempts to use EEADRH on PIC18F4321.
;
; UART Bootloader for PIC18F by Ross Fosler 
; 09/01/2006  Modified to support PIC18xxJxx & 160k PIC18Fxxx Flash Devices
; 03/01/2002 ... First full implementation
; 03/07/2002 Changed entry method to use last byte of EEDATA.
;            Also removed all possible infinite loops w/ clrwdt.
; 03/07/2002 Added code for direct boot entry. I.E. boot vector.
; 03/09/2002 Changed the general packet format, removed the LEN field.
; 03/09/2002 Modified the erase command to do multiple row erase.
; 03/12/2002 Fixed write problem to CONFIG area. Write was offset by a byte.
; 03/15/2002 Added work around for 18F8720 tblwt*+ problem.
; 03/20/2002 Modified receive & parse engine to vector to autobaud on a checksum 
;            error since a chechsum error could likely be a communications problem.
; 03/22/2002 Removed clrwdt from the startup. This instruction affects the TO and 
;            PD flags. Removing this instruction should have no affect on code 
;       operation since the wdt is cleared on a reset and boot entry is always
;       on a reset.
; 03/22/2002    Modified the protocol to incorporate the autobaud as part of the 
;       first received <STX>. Doing this improves robustness by allowing
;       re-sync under any condition. Previously it was possible to enter a 
;       state where only a hard reset would allow re-syncing.
; 03/27/2002    Removed the boot vector and related code. This could lead to customer
;       issues. There is a very minute probability that errent code execution
;       could jump into the boot area and cause artificial boot entry.
; *****************************************************************************
#include <p18cxxx.inc>
#include "configure.inc"
#include "devices.inc"
#include "bootconfig.inc"
#include "preprocess.inc"
; *****************************************************************************

; *****************************************************************************
#define STX             0x0F            
#define ETX             0x04
#define DLE             0x05
#define NTX             0xFF
; *****************************************************************************

; *****************************************************************************
; RAM Address Map
CRCL                equ 0x00
CRCH                equ 0x01
RXDATA              equ 0x02
TXDATA              equ 0x03
TMP		    equ 0x04

; Framed Packet Format
; <STX>[<COMMAND><ADDRL><ADDRH><ADDRU><0x00><DATALEN><...DATA...>]<CRCL><CRCH><ETX>
COMMAND             equ 0x05        ; receive buffer
ADDRESS_L           equ 0x06
ADDRESS_H           equ 0x07
ADDRESS_U           equ 0x08
ADDRESS_X           equ 0x09
DATA_COUNTL         equ 0x0A
PACKET_DATA         equ 0x0B
DATA_COUNTH         equ 0x0B        ; only for certain commands
; *****************************************************************************

; *****************************************************************************
    errorlevel -311                 ; don't warn on HIGH() operator values >16-bits

; bit banding read function
#define INCLK	RA0
#define INDATA	RA1
#define CS	RA2    ; lo=normal, hi=reset
#define OUTDATA	RC6    

ClrOutData macro
    bcf	    PORTC, OUTDATA
    endm
    
SetOutData macro
    bsf	    PORTC, OUTDATA
    endm

ToggleOutData macro
    btg	    PORTC, OUTDATA
    endm
  
#ifdef USE_SOFTBOOTWP
  #ifndef SOFTWP
    #define SOFTWP
  #endif
#endif

#ifdef USE_SOFTCONFIGWP
  #ifdef CONFIG_AS_FLASH
    #ifndef SOFTWP
      #define SOFTWP
    #endif
  #endif
#endif

#ifndef AppVector
    ; The application startup GOTO instruction will be written just before the Boot Block,
    ; courtesy of the host PC bootloader application.
    #define AppVector (BootloaderStart-.4)
#endif
; *****************************************************************************

 
; *****************************************************************************
#if BOOTLOADER_ADDRESS != 0
    ORG     0
    ; The following GOTO is not strictly necessary, but may startup faster
    ; for large microcontrollers running at extremely slow clock speeds.
    ;goto    BootloaderBreakCheck  

    ORG     BOOTLOADER_ADDRESS
BootloaderStart:
    bra     BootloadMode

; *****************************************************************************
; Determine if the application is supposed to be started or if we should
; go into bootloader mode.
;
; If RX pin is in BREAK state when we come out of MCLR reset, immediately 
; enter bootloader mode, even if there exists some application firmware in 
; program memory.
BootloaderBreakCheck:
#ifdef INVERT_UART
    btfss   RXPORT, RXPIN
GotoAppVector:
    goto    AppVector           ; no BREAK state, attempt to start application
#else
    btfsc   RXPORT, RXPIN
GotoAppVector:
    goto    AppVector           ; no BREAK state, attempt to start application
#endif

BootloadMode:
#else ; BOOTLOADER_ADDRESS == 0 ****************************************************************
    ORG     0
BootloaderStart:
    movlw   low(AppVector)      ; load address of application reset vector
    bra     BootloaderBreakCheck

    ORG	    0x0008
HighPriorityInterruptVector:
    goto    AppHighIntVector    ; Re-map Interrupt vector

BootloaderBreakCheck:
    ; TODO: check firmware switch and branch to BootloadMode
    bra	    BootloadMode
    
CheckAppVector:
    ; Read instruction at the application reset vector location. 
    ; If we read 0xFFFF, assume that the application firmware has
    ; not been programmed yet, so don't try going into application mode.
    movwf   TBLPTRL
    movlw   high(AppVector)
    movwf   TBLPTRH
    bra     CheckAppVector2

    ORG	    0x0018
LowPriorityInterruptVector:
    goto    AppLowIntVector     ; Re-map Interrupt vector

CheckAppVector2:
    movlw   upper(AppVector)
    movwf   TBLPTRU     
    tblrd   *+                  ; read instruction from program memory
    incfsz  TABLAT, W           ; if the lower byte != 0xFF, 
GotoAppVector:
    goto    AppVector           ; run application.

    tblrd   *+                  ; read instruction from program memory
    incfsz  TABLAT, W           ; if the lower byte == 0xFF but upper byte != 0xFF,
    bra     GotoAppVector       ; run application.
    ; otherwise, assume application firmware is not present because we read a NOP (0xFFFF).
    ; fall through to bootloader mode...
BootloadMode:
    ; Bootloader entry point
    bsf	    ADCON1, PCFG0	; set RA as digital port PCFG3:PCFG0 = 1111
    bsf	    ADCON1, PCFG1
    bsf	    ADCON1, PCFG2
    bsf	    ADCON1, PCFG3
    
    bsf	    TRISA, TRISA0	; set RA0 as input clock
    bsf	    TRISA, TRISA1	; set RA1 as input data
    bsf	    TRISA, TRISA2	; set RA2 as input cs
    bcf	    TRISC, TRISC6	; set RC6 as output
    bcf	    PORTC, RC6		; set RC6=0

    ; loopback test
#if (0)    
LoopbackMode:
    btfss   PORTA, RA0
    goto    LoopbackMode_1
    bsf	    PORTC, RC6
    bra	    LoopbackMode
LoopbackMode_1:
    bcf	    PORTC, RC6
    bra	    LoopbackMode
#endif
        
#if (0)
LoopbackMode:
    call    DoReadHostByte
    movf    RXDATA, W	; WREG = RXDATA
    movwf   TXDATA	; TXDATA = WREG
    call    DoWriteHostByte
    bra	    LoopbackMode
#endif    
         
#endif ; end BOOTLOADER_ADDRESS == 0 ******************************************
    lfsr    FSR2, 0             ; for compatibility with Extended Instructions mode.

#ifdef USE_MAX_INTOSC
    nop
#endif
    
#ifdef USE_PLL
    ; run internal OSC at 8Mhz,  OSCCON = 0x70
    movlw   0x72
    movwf   OSCCON

    ; enable PLL, OSCTUNE = 0xC0
    movlw   0xC0
    movwf   OSCTUNE
#endif

; *****************************************************************************
; WaitForHostCommand    
DoAutoBaud:
WaitForHostCommand:
    rcall   ReadHostByte        ; get start of transmission <STX>
    xorlw   STX
    bnz     WaitForHostCommand  ; got something unexpected, keep waiting for <STX>
; *****************************************************************************

; -----------------------------------------------------------------------------    
; Notes:    
; Host send the first <STX> then it must wait and receive <STX> from bootloader
; -----------------------------------------------------------------------------    

; *****************************************************************************
; Read and parse packet data.
StartOfLine:
    movlw   STX                     ; send back start of response
    rcall   SendHostByte

    lfsr    FSR0, COMMAND-1         ; Point to the buffer
        
ReceiveDataLoop:
    rcall   ReadHostByte            ; Get the data
    xorlw   STX                     ; Check for an unexpected STX
    bz      StartOfLine             ; unexpected STX: abort packet and start over.

NoSTX:
    movf    RXDATA, W
    xorlw   ETX                     ; Check for a ETX
    bz      VerifyPacketCRC         ; Yes, verify CRC

NoETX:
    movf    RXDATA, W
    xorlw   DLE                     ; Check for a DLE
    bnz     AppendDataBuffer

    rcall   ReadHostByte            ; DLE received, get the next byte and store it
    
AppendDataBuffer:
    movff   RXDATA, PREINC0         ; store the data to the buffer
    bra     ReceiveDataLoop

VerifyPacketCRC:
    lfsr    FSR1, COMMAND
    clrf    CRCL
    clrf    CRCH
    movff   POSTDEC0, PRODH         ; Save host packet's CRCH to PRODH for later comparison
                                    ; CRCL is now available as INDF0
VerifyPacketCrcLoop:
    movf    POSTINC1, w
    rcall   AddCrc                  ; add new data to the CRC

    movf    FSR1H, w
    cpfseq  FSR0H
    bra     VerifyPacketCrcLoop     ; we aren't at the end of the received data yet, loop
    movf    FSR1L, w
    cpfseq  FSR0L
    bra     VerifyPacketCrcLoop     ; we aren't at the end of the received data yet, loop

    movf    CRCH, w
    cpfseq  PRODH
    bra     DoAutoBaud              ; invalid CRC, reset baud rate generator to re-sync with host
    movf    CRCL, w
    cpfseq  INDF0
    bra     DoAutoBaud              ; invalid CRC, reset baud rate generator to re-sync with host

; ***********************************************
; Pre-setup, common to all commands.
    clrf    CRCL
    clrf    CRCH

    movf    ADDRESS_L, W            ; Set all possible pointers
    movwf   TBLPTRL
#ifdef EEADR
    movwf   EEADR
#endif
    movf    ADDRESS_H, W
    movwf   TBLPTRH
#ifdef EEADRH
    movwf   EEADRH
#endif
    movff   ADDRESS_U, TBLPTRU
    lfsr    FSR0, PACKET_DATA
; ***********************************************

 

; ***********************************************
; Test the command field and sub-command.
CheckCommand:
    movlw   .10
    cpfslt  COMMAND
    bra     DoAutoBaud          ; invalid command - reset baud generator to re-sync with host

    ; This jump table must exist entirely within one 256 byte block of program memory.
#if ($ & 0xFF) > (0xFF - .24)
    ; Too close to the end of a 256 byte boundary, push address forward to get code
    ; into the next 256 byte block.
    messg   "Wasting some code space to ensure jump table is aligned."
    ORG     $+(0x100 - ($ & 0xFF))
#endif
JUMPTABLE_BEGIN:
    movf    PCL, w              ; 0 do a read of PCL to set PCLATU:PCLATH to current program counter.
    rlncf   COMMAND, W          ; 2 multiply COMMAND by 2 (each BRA instruction takes 2 bytes on PIC18)
    addwf   PCL, F              ; 4 Jump in command jump table based on COMMAND from host
    bra     BootloaderInfo      ; 6 00h
    bra     ReadFlash           ; 8 01h
    bra     VerifyFlash         ; 10 02h
    bra     EraseFlash          ; 12 03h
    bra     WriteFlash          ; 14 04h
    bra     ReadEeprom          ; 16 05h
    bra     WriteEeprom         ; 18 06h
    bra     WriteConfig         ; 20 07h
    bra     GotoAppVector       ; 22 08h
    reset                       ; 24 09h

#if (JUMPTABLE_BEGIN & 0xFF) > ($ & 0xFF)
    error "Jump table is not aligned to fit within a single 256 byte address range."
#endif
; *****************************************************************************

#ifdef INVERT_UART
WaitForRise:
    nop
    
WaitForRiseLoop:
    btfsc   INTCON, TMR0IF  ; if TMR0 overflowed, we did not get a good baud capture
    return                  ; abort

    btfss   RXPORT, RXPIN   ; Wait for a falling edge
    bra     WaitForRiseLoop

WtSR:
    btfsc   RXPORT, RXPIN   ; Wait for starting edge
    bra     WtSR
    return
#else ; not inverted UART pins
WaitForRise:
    nop

WaitForRiseLoop
    btfsc   INTCON, TMR0IF  ; if TMR0 overflowed, we did not get a good baud capture
    return                  ; abort

    btfsc   RXPORT, RXPIN   ; Wait for a falling edge
    bra     WaitForRiseLoop

WtSR:
    btfss   RXPORT, RXPIN   ; Wait for rising edge
    bra     WtSR
    return
#endif ; end #ifdef INVERT_UART
; *****************************************************************************

; 16-bit CCITT CRC
; Adds WREG byte to the CRC checksum CRCH:CRCL. WREG destroyed on return.
AddCrc:                           ; Init: CRCH = HHHH hhhh, CRCL = LLLL llll
    xorwf   CRCH, w               ; Pre:  HHHH hhhh     WREG =      IIII iiii
    movff   CRCL, CRCH            ; Pre:  LLLL llll     CRCH =      LLLL llll
    movwf   CRCL                  ; Pre:  IIII iiii     CRCL =      IIII iiii
    swapf   WREG                  ; Pre:  IIII iiii     WREG =      iiii IIII
    andlw   0x0F                  ; Pre:  iiii IIII     WREG =      0000 IIII
    xorwf   CRCL, f               ; Pre:  IIII iiii     CRCL =      IIII jjjj
    swapf   CRCL, w               ; Pre:  IIII jjjj     WREG =      jjjj IIII
    andlw   0xF0                  ; Pre:  jjjj IIII     WREG =      jjjj 0000
    xorwf   CRCH, f               ; Pre:  LLLL llll     CRCH =      MMMM llll
    swapf   CRCL, w               ; Pre:  IIII jjjj     WREG =      jjjj IIII
    rlncf   WREG, w               ; Pre:  jjjj IIII     WREG =      jjjI IIIj
    xorwf   CRCH, f               ; Pre:  MMMM llll     CRCH =      XXXN mmmm
    andlw   b'11100000'           ; Pre:  jjjI IIIj     WREG =      jjj0 0000
    xorwf   CRCH, f               ; Pre:  jjj0 0000     CRCH =      MMMN mmmm
    xorwf   CRCL, f               ; Pre:  MMMN mmmm     CRCL =      JJJI jjjj
    return

; ***********************************************
; Commands
; ***********************************************

; Provides information about the Bootloader to the host PC software.
BootInfoBlock:
    db      low(BOOTBLOCKSIZE), high(BOOTBLOCKSIZE)
    db      MAJOR_VERSION, MINOR_VERSION
    db      0xFF, 0x84             ; command mask : family id 
    db      low(BootloaderStart), high(BootloaderStart)
    db      upper(BootloaderStart), 0 
BootInfoBlockEnd:

; In:   <STX>[<0x00>]<CRCL><CRCH><ETX>
; Out:  <STX><BOOTBYTESL><BOOTBYTESH><VERL><VERH><STARTBOOTL><STARTBOOTH><STARTBOOTU><0x00><CRCL><CRCH><ETX>
BootloaderInfo:
    movlw   low(BootInfoBlock)
    movwf   TBLPTRL
    movlw   high(BootInfoBlock)
    movwf   TBLPTRH
    movlw   upper(BootInfoBlock)
    movwf   TBLPTRU

    movlw   (BootInfoBlockEnd - BootInfoBlock)
    movwf   DATA_COUNTL
    clrf    DATA_COUNTH
    ;; fall through to ReadFlash code -- send Bootloader Information Block from FLASH.

; In:   <STX>[<0x01><ADDRL><ADDRH><ADDRU><0x00><BYTESL><BYTESH>]<CRCL><CRCH><ETX>
; Out:  <STX>[<DATA>...]<CRCL><CRCH><ETX>
ReadFlash:
    tblrd   *+                  ; read from FLASH memory into TABLAT
    movf    TABLAT, w
    rcall   SendEscapeByte
    rcall   AddCrc

    decf    DATA_COUNTL, f      ; decrement counter
    movlw   0
    subwfb  DATA_COUNTH, f

    movf    DATA_COUNTL, w      ; DATA_COUNTH:DATA_COUNTH == 0?
    iorwf   DATA_COUNTH, w
    bnz     ReadFlash           ; no, loop
    bra     SendChecksum        ; yes, send end of packet

; In:   <STX>[<0x02><ADDRL><ADDRH><ADDRU><0x00><BLOCKSL><BLOCKSH>]<CRCL><CRCH><ETX>
; Out:  <STX>[<CRCL1><CRCH1>...<CRCLn><CRCHn>]<ETX>
VerifyFlash:
    tblrd   *+
    movf    TABLAT, w    
    rcall   AddCrc

    movf    TBLPTRL, w          ; have we crossed into the next block?
#if ERASE_FLASH_BLOCKSIZE > .255
    bnz     VerifyFlash
    movf    TBLPTRH, w
    andlw   high(ERASE_FLASH_BLOCKSIZE-1)
#else
    andlw   (ERASE_FLASH_BLOCKSIZE-1)    
#endif
    bnz     VerifyFlash

    movf    CRCL, w
    call    SendEscapeByte
    movf    CRCH, w
    call    SendEscapeByte

    decf    DATA_COUNTL, f      ; decrement counter
    movlw   0
    subwfb  DATA_COUNTH, f

    movf    DATA_COUNTL, w      ; DATA_COUNTH:DATA_COUNTH == 0?
    iorwf   DATA_COUNTH, w
    bnz     VerifyFlash         ; no, loop
    bra     SendETX             ; yes, send end of packet

#ifdef SOFTWP
    reset                       ; this code -should- never be executed, but 
    reset                       ; just in case of errant execution or buggy
    reset                       ; firmware, these reset instructions may protect
    reset                       ; against accidental erases.
#endif

; In:   <STX>[<0x03><ADDRL><ADDRH><ADDRU><0x00><PAGESL>]<CRCL><CRCH><ETX>
; Out:  <STX>[<0x03>]<CRCL><CRCH><ETX>
EraseFlash:
#ifdef SOFTWP
  #define ERASE_ADDRESS_MASK  (~(ERASE_FLASH_BLOCKSIZE-1))
  #if upper(ERASE_ADDRESS_MASK) != 0xFF
    movlw   upper(ERASE_ADDRESS_MASK)    ; force starting address to land on a FLASH Erase Block boundary
    andwf   TBLPTRU, f
  #endif
  #if high(ERASE_ADDRESS_MASK) != 0xFF
    movlw   high(ERASE_ADDRESS_MASK)    ; force starting address to land on a FLASH Erase Block boundary
    andwf   TBLPTRH, f
  #endif
  #if low(ERASE_ADDRESS_MASK) != 0xFF
    movlw   low(ERASE_ADDRESS_MASK)     ; force starting address to land on a FLASH Erase Block boundary
    andwf   TBLPTRL, f
  #endif

    ; Verify Erase Address does not attempt to erase beyond the end of FLASH memory
    movlw   low(END_FLASH)
    subwf   TBLPTRL, w
    movlw   high(END_FLASH)
    subwfb  TBLPTRH, w
    movlw   upper(END_FLASH)
    subwfb  TBLPTRU, w
    bn      EraseEndFlashAddressOkay

    clrf    EECON1              ; inhibit writes for this block
    bra     NextEraseBlock      ; move on to next erase block
#endif ; end #ifdef USE_SOFTBOOTWP

EraseEndFlashAddressOkay:
#ifdef USE_SOFTCONFIGWP
    #ifdef CONFIG_AS_FLASH
    movlw   low(END_FLASH - ERASE_FLASH_BLOCKSIZE)
    subwf   TBLPTRL, w
    movlw   high(END_FLASH - ERASE_FLASH_BLOCKSIZE)
    subwfb  TBLPTRH, w
    movlw   upper(END_FLASH - ERASE_FLASH_BLOCKSIZE)
    subwfb  TBLPTRU, w
    bn      EraseConfigAddressOkay

    clrf    EECON1              ; inhibit writes for this block
    bra     NextEraseBlock      ; move on to next erase block

EraseConfigAddressOkay:
    #endif ; end CONFIG_AS_FLASH
#endif ; end USE_SOFTCONFIGWP

#ifdef USE_SOFTBOOTWP
    movlw   low(BOOTLOADER_ADDRESS)
    subwf   TBLPTRL, w
    movlw   high(BOOTLOADER_ADDRESS)
    subwfb  TBLPTRH, w
    movlw   upper(BOOTLOADER_ADDRESS)
    subwfb  TBLPTRU, w
    bn      EraseAddressOkay

    movlw   low(BOOTLOADER_ADDRESS + BOOTBLOCKSIZE)
    subwf   TBLPTRL, w
    movlw   high(BOOTLOADER_ADDRESS + BOOTBLOCKSIZE)
    subwfb  TBLPTRH, w
    movlw   upper(BOOTLOADER_ADDRESS + BOOTBLOCKSIZE)
    subwfb  TBLPTRU, w
    bnn     EraseAddressOkay

    clrf    EECON1              ; inhibit writes for this block
    bra     NextEraseBlock      ; move on to next erase block

    reset                       ; this code -should- never be executed, but 
    reset                       ; just in case of errant execution or buggy
    reset                       ; firmware, these reset instruction may protect
    reset                       ; against accidental writes.
#endif

EraseAddressOkay:
#ifdef EEADR
    movlw   b'10010100'         ; setup FLASH erase
#else
    movlw   b'00010100'         ; setup FLASH erase for J device (no EEPROM bit)
#endif
    movwf   EECON1

    rcall   StartWrite          ; erase the page

NextEraseBlock:
    ; Decrement address by erase block size
#if ERASE_FLASH_BLOCKSIZE >= .256
    movlw   high(ERASE_FLASH_BLOCKSIZE)
    subwf   TBLPTRH, F
    clrf    WREG
    subwfb  TBLPTRU, F
#else
    movlw   ERASE_FLASH_BLOCKSIZE
    subwf   TBLPTRL, F
    clrf    WREG
    subwfb  TBLPTRH, F
    subwfb  TBLPTRU, F
#endif

    decfsz  DATA_COUNTL, F
    bra     EraseFlash    
    bra     SendAcknowledge     ; All done, send acknowledgement packet

#ifdef SOFTWP
    reset                       ; this code -should- never be executed, but 
    reset                       ; just in case of errant execution or buggy
    reset                       ; firmware, these reset instructions may protect
    reset                       ; against accidental writes.
#endif

; In:   <STX>[<0x04><ADDRL><ADDRH><ADDRU><0x00><BLOCKSL><DATA>...]<CRCL><CRCH><ETX>
; Out:  <STX>[<0x04>]<CRCL><CRCH><ETX>
WriteFlash:
#ifdef SOFTWP
  #define WRITE_ADDRESS_MASK (~(WRITE_FLASH_BLOCKSIZE-1))
  #if upper(WRITE_ADDRESS_MASK) != 0xFF
    movlw   upper(WRITE_ADDRESS_MASK)    ; force starting address to land on a FLASH Write Block boundary
    andwf   TBLPTRU, f
  #endif
  #if high(WRITE_ADDRESS_MASK) != 0xFF
    movlw   high(WRITE_ADDRESS_MASK)    ; force starting address to land on a FLASH Write Block boundary
    andwf   TBLPTRH, f
  #endif
  #if low(WRITE_ADDRESS_MASK) != 0xFF
    movlw   low(WRITE_ADDRESS_MASK)     ; force starting address to land on a FLASH Write Block boundary
    andwf   TBLPTRL, f
  #endif

    ; Verify Write Address does not attempt to write beyond the end of FLASH memory
    movlw   low(END_FLASH)
    subwf   TBLPTRL, w
    movlw   high(END_FLASH)
    subwfb  TBLPTRH, w
    movlw   upper(END_FLASH)
    subwfb  TBLPTRU, w
    bn      WriteEndFlashAddressOkay

    clrf    EECON1              ; inhibit writes for this block
    bra     LoadHoldingRegisters; fake the write so we can move on to real writes
#endif ; end #ifdef SOFTWP

WriteEndFlashAddressOkay:
#ifdef USE_SOFTCONFIGWP
    #ifdef CONFIG_AS_FLASH
    movlw   low(END_FLASH - ERASE_FLASH_BLOCKSIZE)
    subwf   TBLPTRL, w
    movlw   high(END_FLASH - ERASE_FLASH_BLOCKSIZE)
    subwfb  TBLPTRH, w
    movlw   upper(END_FLASH - ERASE_FLASH_BLOCKSIZE)
    subwfb  TBLPTRU, w
    bn      WriteConfigAddressOkay

    clrf    EECON1              ; inhibit writes for this block
    bra     LoadHoldingRegisters; fake the write so we can move on to real writes

WriteConfigAddressOkay:
    #endif ; end CONFIG_AS_FLASH
#endif ; end USE_SOFTCONFIGWP

#ifdef USE_SOFTBOOTWP
    movlw   low(BOOTLOADER_ADDRESS)
    subwf   TBLPTRL, w
    movlw   high(BOOTLOADER_ADDRESS)
    subwfb  TBLPTRH, w
    movlw   upper(BOOTLOADER_ADDRESS)
    subwfb  TBLPTRU, w
    bn      WriteAddressOkay

    movlw   low(BOOTLOADER_ADDRESS + BOOTBLOCKSIZE)
    subwf   TBLPTRL, w
    movlw   high(BOOTLOADER_ADDRESS + BOOTBLOCKSIZE)
    subwfb  TBLPTRH, w
    movlw   upper(BOOTLOADER_ADDRESS + BOOTBLOCKSIZE)
    subwfb  TBLPTRU, w
    bnn     WriteAddressOkay

    clrf    EECON1                      ; inhibit writes for this block
    bra     LoadHoldingRegisters        ; fake the write so we can move on to real writes

    reset                       ; this code -should- never be executed, but 
    reset                       ; just in case of errant execution or buggy
    reset                       ; firmware, these reset instruction may protect
    reset                       ; against accidental writes.
#endif

WriteAddressOkay:
#ifdef EEADR
    movlw   b'10000100'         ; Setup FLASH writes
#else
    movlw   b'00000100'         ; Setup FLASH writes for J device (no EEPROM bit)
#endif
    movwf   EECON1

LoadHoldingRegisters:
    movff   POSTINC0, TABLAT    ; Load the holding registers
    pmwtpi                      ; Same as tblwt *+

    movf    TBLPTRL, w          ; have we crossed into the next write block?
    andlw   (WRITE_FLASH_BLOCKSIZE-1)
    bnz     LoadHoldingRegisters; Not finished writing holding registers, repeat

    tblrd   *-                  ; Point back into the block to write data
    rcall   StartWrite          ; initiate a page write
    tblrd   *+                  ; Restore pointer for loading holding registers with next block

    decfsz  DATA_COUNTL, F      
    bra     WriteFlash          ; Not finished writing all blocks, repeat
    bra     SendAcknowledge     ; all done, send ACK packet

; In:   <STX>[<0x05><ADDRL><ADDRH><0x00><0x00><BYTESL><BYTESH>]<CRCL><CRCH><ETX>
; Out:  <STX>[<DATA>...]<CRCL><CRCH><ETX>
#ifdef EEADR                ; some devices do not have EEPROM, so no need for this code
ReadEeprom:
    clrf    EECON1 
ReadEepromLoop:
    bsf     EECON1, RD          ; Read the data
    movf    EEDATA, w
    #ifdef EEADRH
    infsnz  EEADR, F            ; Adjust EEDATA pointer
    incf    EEADRH, F
    #else
    incf    EEADR, F            ; Adjust EEDATA pointer
    #endif
    rcall   SendEscapeByte
    rcall   AddCrc

    #ifdef EEADRH
    decf    DATA_COUNTL, f      ; decrement counter
    movlw   0
    subwfb  DATA_COUNTH, f

    movf    DATA_COUNTL, w      ; DATA_COUNTH:DATA_COUNTH == 0?
    iorwf   DATA_COUNTH, w
    bnz     ReadEepromLoop      ; no, loop
    bra     SendChecksum        ; yes, send end of packet
    #else
    decfsz  DATA_COUNTL, F
    bra     ReadEepromLoop      ; Not finished then repeat
    bra     SendChecksum
    #endif
#endif ; end #ifdef EEADR

; In:   <STX>[<0x06><ADDRL><ADDRH><0x00><0x00><BYTESL><BYTESH><DATA>...]<CRCL><CRCH><ETX>
; Out:  <STX>[<0x06>]<CRCL><CRCH><ETX>
#ifdef EEADR                ; some devices do not have EEPROM, so no need for this code
WriteEeprom:
    movlw   b'00000100'     ; Setup for EEPROM data writes
    movwf   EECON1

WriteEepromLoop:
    movff   PREINC0, EEDATA
    rcall   StartWrite      

    btfsc   EECON1, WR      ; wait for write to complete before moving to next address
    bra     $-2

    #ifdef EEADRH
    infsnz  EEADR, F        ; Adjust EEDATA pointer
    incf    EEADRH, F
    #else
    incf    EEADR, f        ; Adjust EEDATA pointer
    #endif

    #ifdef EEADRH
    decf    DATA_COUNTL, f      ; decrement counter
    movlw   0
    subwfb  DATA_COUNTH, f

    movf    DATA_COUNTL, w      ; DATA_COUNTH:DATA_COUNTH == 0?
    iorwf   DATA_COUNTH, w
    bnz     WriteEepromLoop     ; no, loop
    bra     SendAcknowledge     ; yes, send end of packet
    #else
    decfsz  DATA_COUNTL, f
    bra     WriteEepromLoop
    bra     SendAcknowledge
    #endif
#endif ; end #ifdef EEADR
 
; In:   <STX>[<0x07><ADDRL><ADDRH><ADDRU><0x00><BYTES><DATA>...]<CRCL><CRCH><ETX>
; Out:  <STX>[<0x07>]<CRCL><CRCH><ETX>
#ifndef CONFIG_AS_FLASH     ; J flash devices store config words in FLASH, so no need for this code
    #ifndef USE_SOFTCONFIGWP
WriteConfig:
    movlw   b'11000100'
    movwf   EECON1
    tblrd   *               ; read existing value from config memory

WriteConfigLoop:
    movf    POSTINC0, w
    cpfseq  TABLAT          ; is the proposed value already the same as existing value?
    rcall   TableWriteWREG  ; write config memory only if necessary (save time and endurance)
    tblrd   +*              ; increment table pointer to next address and read existing value
    decfsz  DATA_COUNTL, F
    bra     WriteConfigLoop ; If more data available in packet, keep looping

    bra     SendAcknowledge ; Send acknowledge
    #endif ; end #ifndef USE_SOFTCONFIGWP
#endif ; end #ifndef CONFIG_AS_FLASH
    
;************************************************

; ***********************************************
; Send an acknowledgement packet back
;
; <STX><COMMAND><CRCL><CRCH><ETX>

; Some devices only have config words as FLASH memory. Some devices don't have EEPROM.
; For these devices, we can save code by jumping directly to sending back an
; acknowledgement packet if the PC application erroneously requests them.
#ifdef CONFIG_AS_FLASH
WriteConfig:
#else
  #ifdef USE_SOFTCONFIGWP
WriteConfig:
  #endif
#endif ; end #ifdef CONFIG_AS_FLASH

#ifndef EEADR
ReadEeprom:
WriteEeprom:
#endif

SendAcknowledge:
    clrf    EECON1              ; inhibit write cycles to FLASH memory

    movf    COMMAND, w
    rcall   SendEscapeByte      ; Send only the command byte (acknowledge packet)
    rcall   AddCrc

SendChecksum:
    movf    CRCL, W
    rcall   SendEscapeByte

    movf    CRCH, W
    rcall   SendEscapeByte

SendETX:
    movlw   ETX             ; Send stop condition
    rcall   SendHostByte

    bra     WaitForHostCommand
; *****************************************************************************

; *****************************************************************************
; Write a byte to the serial port while escaping control characters with a DLE
; first.
SendEscapeByte:
    movwf   TXDATA          ; Save the data
 
    xorlw   STX             ; Check for a STX
    bz      WrDLE           ; No, continue WrNext

    movf    TXDATA, W       
    xorlw   ETX             ; Check for a ETX
    bz      WrDLE           ; No, continue WrNext

    movf    TXDATA, W       
    xorlw   DLE             ; Check for a DLE
    bnz     WrNext          ; No, continue WrNext

WrDLE:
    movlw   DLE             ; Yes, send DLE first
    bra	    SendHostDLEByte

WrNext:
    movf    TXDATA, W       ; Then send STX
    bra	    SendHostByte
    
SendHostDLEByte:
    call    DoWriteHostByte
    movf    TXDATA, W
    
SendHostByte:
    call    DoWriteHostByte
    return
        
; *****************************************************************************
; Host must set the INDATA before rising INCLK    
DoReadHostBit:
    rrncf   RXDATA, 1		; RXDATA = RXDATA >> 1;
a1:    
    btfsc   PORTA, CS
    reset
    btfss   PORTA, INCLK	; wait clock go high
    bra	    a1
    btfss   PORTA, INDATA	; if INDATA == 1 then set RXDATA
    goto    a2
    bsf	    RXDATA, 7		; set RXDATA's bit 7 to 1
a2:
    btfsc   PORTA, CS
    reset
    btfsc   PORTA, INCLK	; wait clock go low
    bra	    a2
    return
    
; Host need to wait until OutData go high
; Host put data on InData, clock high, wait OutData to toggle, clock low
DoReadHostByte:
    clrf    RXDATA
    ; Ready to receive
    call DoReadHostBit ; bit 0
    call DoReadHostBit ; bit 1
    call DoReadHostBit ; bit 2
    call DoReadHostBit ; bit 3
    call DoReadHostBit ; bit 4
    call DoReadHostBit ; bit 5
    call DoReadHostBit ; bit 6
    call DoReadHostBit ; bit 7
    ; W reg = contents of RXDATA
    movf    RXDATA, W
    ; Not ready to receive
    ClrOutData
    return

; *****************************************************************************
DoWriteHostBit:
    ; check CS and clock high
b1:    
    btfsc   PORTA, CS
    reset
    btfss   PORTA, INCLK
    bra	    b1
    ; shift right TMP thru carry
    rrcf    TMP, 1
    bc	    setbit
clrbit:
    ClrOutData
    goto    b2
setbit:
    SetOutData
b2:    
    ; check CS and clock low
    btfsc   PORTA, CS
    reset
    btfsc   PORTA, INCLK
    bra	    b2
    return
 
; Host need to wait until OutData is high
; Then set clock high, wait 1 ms, read OutData, clock low for 1 ms 
DoWriteHostByte:
    ; Tx data ready
    SetOutData
    ; Tx data
    movwf   TMP	; TMP = WREG
    call    DoWriteHostBit ; bit 0
    call    DoWriteHostBit ; bit 1
    call    DoWriteHostBit ; bit 2
    call    DoWriteHostBit ; bit 3
    call    DoWriteHostBit ; bit 4
    call    DoWriteHostBit ; bit 5
    call    DoWriteHostBit ; bit 6
    call    DoWriteHostBit ; bit 7
    ; Tx data done
    ClrOutData
    return

; *****************************************************************************
ReadHostByte:
    call    DoReadHostByte
    return
; *****************************************************************************

    reset                       ; this code -should- never be executed, but 
    reset                       ; just in case of errant execution or buggy
    reset                       ; firmware, these instructions may protect
    clrf    EECON1              ; against accidental erase/write operations.

; *****************************************************************************
; Unlock and start the write or erase sequence.
TableWriteWREG:
    movwf   TABLAT
    tblwt   *

StartWrite:
    movlw   0x55            ; Unlock
    movwf   EECON2
    movlw   0xAA
    movwf   EECON2
    bsf     EECON1, WR      ; Start the write
    nop

    return
; *****************************************************************************

    END
