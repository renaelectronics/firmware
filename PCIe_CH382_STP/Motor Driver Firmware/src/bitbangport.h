#ifndef BITBANGPORT_H
#define	BITBANGPORT_H

/* bit banging pin defination */
#define HOST_INCLK      PORTAbits.RA0
#define HOST_INDATA     PORTAbits.RA1
#define HOST_CS         PORTAbits.RA2 /* lo=normal, hi=reset */
#define HOST_OUTDATA	PORTCbits.RC6    

void ClrOutData(void);
void SetOutData(void);

unsigned char DoWriteHostBit(unsigned char* pdata);
unsigned char DoWriteHostByte(unsigned char data);

unsigned char DoReadHostBit(unsigned char *pdata);
unsigned char DoReadHostByte(unsigned char *pdata);

#endif	/* BITBANGPORT_H */

