	LIST P=12F617, R=DEC

#include p12F617.inc
GPRAM			EQU 32

d1              EQU	GPRAM+0
d2              EQU	GPRAM+1

LED_PIN         EQU 2           ; GP2
RX_PIN          EQU 5           ; GP5

	ORG	0
	goto	Start               ; this instruction gets moved to address 0x63C automatically
                                ; by the AN1310ui application so that the bootloader firmware
                                ; can be started first instead.

;--------------------------------------------------------------------------
; Main Program
;--------------------------------------------------------------------------
    ORG 4
Start:
    clrf	GPIO				; initialize port data latches

    BANKSEL ANSEL
    clrf    ANSEL       ; make all analog pins act as digital inputs
    bcf     TRISIO, LED_PIN
    bsf     WPU, RX_PIN
    BANKSEL GPIO

ToggleLoop:
    bsf     GPIO, 2
    call    Wait
    bcf     GPIO, 2
    call    Wait

    btfsc   GPIO, RX_PIN
    goto    ToggleLoop

EndApplication:
    clrf    PCLATH
    goto    0

Wait:
    clrf    d1
    clrf    d2
WaitLoop:
    nop
    nop
    nop
    nop
    nop
    incfsz  d1, f
    goto    WaitLoop
    incfsz  d2, f
    goto    WaitLoop
    return


    END