;CONFIG1H
    config OSC = INTIO2     ; Oscillator (Internal oscillator block, port function on RA6 and RA7)
    config FCMEN = OFF      ; Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
    config IESO = OFF       ; Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

; CONFIG2L
    config PWRT = ON        ; Power-up Timer Enable bit (PWRT enabled)
    config BOR = ON         ; Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
    config BORV = 2         ; Brown-out Reset Voltage bits ()

; CONFIG2H
    config WDT = OFF        ; Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
    config WDTPS = 32768    ; Watchdog Timer Postscale Select bits (1:32768)

; CONFIG3H
    config CCP2MX = RC1     ; CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
    config PBADEN = DIG     ; PORTB A/D Enable bit (PORTB<4:0> pins areconfigured as digital I/O on Reset)
    config LPT1OSC = OFF    ; Low-Power Timer1 Oscillator Enable bit (Timer1configured for higher power operation)
    config MCLRE = OFF      ; MCLR Pin Enable bit (RE3 input pin enabled; MCLR disabled)

; CONFIG4L
    config STVREN = OFF     ; Stack Full/Underflow Reset Enable bit (Stack full/underflow will not cause Reset)
    config LVP = OFF        ; Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
    config BBSIZ = BB512    ; Boot Block Size Select bits
    config XINST = OFF      ; Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

; CONFIG5L
    config CP0 = OFF        ; Code Protection bit 
    config CP1 = OFF        ; Code Protection bit 

; CONFIG5H
    config CPB = OFF        ; Boot Block Code Protection bitProtect Boot 
    config CPD = OFF        ; Data EEPROM Code Protection bit (Data EEPROM not code-protected)

; CONFIG6L
    config WRT0 = OFF       ; Write Protection bit (Block 0 not write-protected)
    config WRT1 = OFF       ; Write Protection bit (Block 1 not write-protected)

; CONFIG6H
    config WRTC = ON        ; Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) write-protected)
    config WRTB = ON        ; Boot Block Write Protection bit (Boot block not write-protected)
    config WRTD = OFF       ; Data EEPROM Write Protection bit (Data EEPROM not write-protected)

; CONFIG7L
    config EBTR0 = OFF      ; Table Read Protection bit (Block 0 not protected from table reads executed in other blocks)
    config EBTR1 = OFF      ; Table Read Protection bit (Block 1 not protected from table reads executed in other blocks)

; CONFIG7H
    config EBTRB = OFF      ; Boot Block Table Read Protection bit (Boot block not protected from table reads executed in other blocks)

