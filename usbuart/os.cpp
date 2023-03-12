#include <xc.h>

void poll(unsigned), CDCTick(void);

#define MS 40000

void init(void) {
  __builtin_disable_interrupts();
  _CP0_SET_COMPARE(MS);
  SYSKEY = 0;                       // ensure OSCCON is locked
  SYSKEY = 0xAA996655;              // unlock sequence
  SYSKEY = 0x556699AA;
  CFGCONbits.IOLOCK = 0;            // allow write
  U1RXR = 0b0100;                   // PB2    (uart1)
  RPB3R = 0b0001;                   // U1TX
  U2RXR = 0b0010;                   // PB1    (uart2 for debug)
  RPB0R = 0b0010;                   // U2TX
  CFGCONbits.IOLOCK = 1;            // forbidden write
  SYSKEY = 0;                       // relock
  ANSELBCLR = 0xf;
  TRISBCLR = 0x200;                 // RB9
  U1BRG = U2BRG = 21;               // UART1 - baud rate = 115200
  U2STAbits.UTXEN = 1;
  U1STASET = 0x1400;                // UART1 - URXEN & UTXEN
  IPC0bits.CTIP = 1;
  IEC0bits.CTIE = 1;
  IPC8bits.U1IP = 1;
  IEC1bits.U1RXIE = 1;
  U1MODEbits.ON = 1;
  U2MODEbits.ON = 1;
  INTCONSET = _INTCON_MVEC_MASK;
  __builtin_enable_interrupts();
}

namespace {
    
    volatile unsigned tick;
    
    void loop(unsigned t) {
        static unsigned tick;
        if (tick == t) return;
        poll(tick = t);
    }
    
}//anonymous

void wait(unsigned i) {
    unsigned u = tick;
    for (loop(u); i--; u = tick) while (u == tick) loop(u);
}

extern "C"
void __attribute__((interrupt(ipl1soft), vector(_CORE_TIMER_VECTOR), nomips16)) rtcISR(void) {
  _CP0_SET_COMPARE(_CP0_GET_COMPARE() + MS);
  tick++;
  CDCTick();
  IFS0bits.CTIF = 0;
}

void toggle(void) { LATBINV = 0x200; }

