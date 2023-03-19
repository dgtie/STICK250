#include <xc.h>

void poll(unsigned);

#define MS 20000	// 1 ms

void init(void) {
  __builtin_disable_interrupts();
  _CP0_SET_COMPARE(MS);
  SYSKEY = 0;                       // ensure OSCCON is locked
  SYSKEY = 0xAA996655;              // unlock sequence
  SYSKEY = 0x556699AA;
  CFGCONbits.IOLOCK = 0;            // allow write
  U1RXR = 0b0100;                   // PB2
  RPB3R = 0b0001;                   // U1TX
  CFGCONbits.IOLOCK = 1;            // forbidden write
  SYSKEY = 0;                       // relock
  ANSELBCLR = 0xC;
  TRISBCLR = 0x200;                 // RB9
  U1BRG = 21;                       // UART1 - baud rate = 115200
  U1STASET = 0x1400;                // UART1 - URXEN & UTXEN
  IPC0bits.CTIP = 1;
  IEC0bits.CTIE = 1;
  IPC8bits.U1IP = 1;
  IEC1bits.U1RXIE = 1;
  U1MODEbits.ON = 1;
  INTCONSET = _INTCON_MVEC_MASK;
  __builtin_enable_interrupts();
}

namespace {
    
    volatile unsigned tick;
    
    void loop(unsigned t) {
        static unsigned tick;
        if (tick != t) poll(tick = t);
    }
    
}//anonymous

bool wait(unsigned i) {
    unsigned u = tick;
    for (loop(u); i--; u = tick) while (u == tick) loop(u);
    return true;
}

void bridge_poll(void);
extern "C"
void __attribute__((interrupt(ipl1soft), vector(_CORE_TIMER_VECTOR), nomips16)) rtcISR(void) {
  _CP0_SET_COMPARE(_CP0_GET_COMPARE() + MS);
  tick++;
  bridge_poll();
  IFS0bits.CTIF = 0;
}

void toggle_LED(void) { LATBINV = 0x200; }

void on_switch_change(bool);
void read_switch(void) {
  static int state = 0x100;	// assume switch is released
  if (state != (PORTB & 0x100)) on_switch_change(state ^= 0x100);
}
