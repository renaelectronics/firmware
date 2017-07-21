/*
 * File:   bitbangport.c
 * Author: ThomasTai
 *
 * Created on July 2, 2017, 4:53 PM
 */
#define USE_OR_MASKS
#include <p18cxxx.h>
#include <stdio.h>
#include <stdlib.h>
#include "spi.h"
#include "io.h"
#include "interrupt.h"
#include "eeprom.h"
#include "L6474.h"
#include "bitbangport.h"

/* variables */
unsigned char bitbangctr;
unsigned char bitbangtxdata;

/* functions */
void ClrOutData(void){
    HOST_OUTDATA = 0;
}
void SetOutData(void){
    HOST_OUTDATA = 1;
}

/* set OutData base on bit 0 of pdata then shift pdata right once  */
unsigned char DoWriteHostBit(unsigned char* pdata){
    /* check CS, clock high */
    for(;;){
        if (HOST_CS || (MX_ENABLE==0))
            return 0;
        if (HOST_INCLK)
            break;
    }
    /* clear or set outdata base on pdata bit 0 */
    if (*pdata & 0x01)
        SetOutData();
    else
        ClrOutData();
    /* shift data to right once */
    *pdata = (unsigned char)(*pdata >> 1);
    for(;;){
        if (HOST_CS || (MX_ENABLE==0))
            return 0;
        if (!HOST_INCLK)
            break;
    }
    return 1;
}
 
/* Host need to wait until OutData is high
 * Then set clock high, wait 1 ms, read OutData, clock low for 1 ms 
 */
unsigned char DoWriteHostByte(unsigned char data){
    bitbangtxdata = data;
    /* Tx data ready */
    SetOutData();
    /* Tx data */
    for (bitbangctr=0; bitbangctr<8; bitbangctr++){
        if  (DoWriteHostBit(&bitbangtxdata) == 0){
            ClrOutData();
            return 0;
        }
    }
    /* Tx data done */
    ClrOutData();
    return 1;
}

/* Host must set the INDATA before rising INCLK */
unsigned char DoReadHostBit(unsigned char *pdata){
    /* shift data right once */
    *pdata = (unsigned char)(*pdata >> 1);
    for(;;){
        if (HOST_CS || (MX_ENABLE==0))
            return 0;
        if (HOST_INCLK)
            break;
    }
    /* set or clear the *pdata */
    if (HOST_INDATA == 1)
        *pdata = (unsigned char)(*pdata | 0x80);
    else
        *pdata = (unsigned char)(*pdata & 0x7f);
    /* check CS, clock low */
    for(;;){
        if (HOST_CS || (MX_ENABLE==0))
            return 0;
        if (!HOST_INCLK)
            break;
    }
    return 1;
}
    
/* Host need to wait until OutData go high
 * Host put data on InData, clock high, wait OutData to toggle, clock low
 */ 
unsigned char DoReadHostByte(unsigned char *pdata){
    *pdata = 0;
    /* clear data out pin */
    ClrOutData();
    for (bitbangctr=0; bitbangctr<8; bitbangctr++){
        if (DoReadHostBit(pdata) == 0){
            ClrOutData();
            return 0;
        }
    }
    ClrOutData();
    return 1;
}
     
/* Put an 8 bit HEX number to bit bang port */
void bitbang_p2x(char input_byte) {
    unsigned char ascii_value[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'a', 'b', 'c', 'd', 'e', 'f'
    };
    DoWriteHostByte(ascii_value[(unsigned char)((input_byte >> 4) & 0xf)]);
    DoWriteHostByte(ascii_value[(unsigned char)(input_byte & 0xf)]);
}

void bitbang_p8x(unsigned long value) {
    bitbang_p2x((value >> 24) & 0xff);
    bitbang_p2x((value >> 16) & 0xff);
    bitbang_p2x((value >> 8) & 0xff);
    bitbang_p2x(value & 0xff);
}