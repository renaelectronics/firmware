; Copyright (c) 2009,  Microchip Technology Inc.
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
; E. Schlunder  08/06/2009  Created replacement vector code to facilitate
;                           remapping PIC16 reset and interrupt vectors 
;                           for HI-TECH C applications running under
;                           a bootloader located at program memory 
;                           address 0000h.
;
; NOTE: This special vector remapping code should NOT be used on 
;       PIC16 Enhanced Core devices (such as PIC16F193X), as the
;       enhanced core provides automatic hardware save/restore of 
;       interrupt context. Simply remove this assembly file from
;       your project when compiling for enhanced core devices.

#include <aspic.h>

GLOBAL _PCLATH_TEMP
GLOBAL _W_TEMP
GLOBAL start

GLOBAL AppVector
FNROOT AppVector

PSECT AppVector,global,class=CODE,delta=2,abs

    ORG 0x400                       ; Change this number to match bootloader AppVector definition
AppVector:
#if (start & 0xFF00) != (AppVector & 0xFF00)
    movlw   (start>>8)
    movwf   PCLATH
    goto    start
#else
    goto    start
#endif

PSECT reset_vec,global,class=CODE,delta=2,ovrld
GLOBAL AppIntVector
FNROOT AppIntVector
AppIntVector:
    swapf   _PCLATH_TEMP, w         ; Bx read PCLATH from bootloader temporary variable
    movwf   PCLATH                  ; Bx restore PCLATH register
    swapf   _W_TEMP, f              ; Bx swap W_TEMP so we can restore W_TEMP into W register
    swapf   _W_TEMP, w              ; Bx (SWAPF's used to avoid damaging STATUS register)

    ; HI-TECH C generated interrupt entry code (psect intentry) will follow immediately here. 
    ; It will save and restore the W, STATUS, and PCLATH registers once again, plus any other
    ; context variables needed for the C interrupt routine. 
