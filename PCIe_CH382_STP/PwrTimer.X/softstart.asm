#include p10f200.inc

    __CONFIG _MCLRE_OFF & _CP_OFF & _WDT_OFF & _IntRC_OSC
    
    CBLOCK 0x10         ; Define all user varables starting at location 0x7
        tick_10
        tick_31
    endc

	org 0x0             ; Reset vector is at 0x0000
	movlw	b'11000111'	; Option Register Value
;             1.......  ; Disable Wake-up on Pin Change
;             .1......  ; Disable Weak Pull-ups bit GP0,1,3
;             ..0.....  ; T0CS Timer0 Clock from Fosc/4
;             ...0....  ; T0SE clock edge low-to-high
;             ....0...  ; Prescaler assign to Timer0
;             .....111  ; Prescaler rate 1:256
	option              ; load OPTION register

startup                 ; Start-up and initialization code starts here
	clrf	GPIO		; Precondition GPIO output latch to 0
	movlw 	b'11111001'	; Make GP1 and GP2 as output others as input
	tris	GPIO 		; Re-initialize port direction control register
	clrwdt              ; clear watchdog
    bcf     GPIO, 1     ; LED = 0
    bcf     GPIO, 2     ; turn off relay

    movlw   .10         ; 10 x 1s = 10s
    movwf   tick_10

wait_10sec
    movlw   .31         ; 31 x 32ms = 1s
    movwf   tick_31

wait_sec
    clrf   TMR0         ; TMR0 = 0

wait_32ms               ; 1 tick is 0.256ms
    btfss   TMR0, 7     ; test bit 7, skip if set
    goto    wait_32ms   ; wait 128 x 0.256ms = 32ms

    decfsz  tick_31, 1  ; tick_31 = 1s
    goto    wait_sec

    btfsc   tick_10, 0  ; toggle LED
    bsf     GPIO, 1
    btfss   tick_10, 0
    bcf     GPIO, 1

    decfsz  tick_10, 1  ; tick_10 = 10s
    goto    wait_10sec

    bsf     GPIO, 2     ; turn on relay
    bcf     GPIO, 1     ; turn on LED

loop
	goto	loop
	end
