#include <xc.h>

void poll(unsigned), button_changed(int);

#define M25 5000  // 0.25ms

void init(void) {
  __builtin_disable_interrupts();
  _CP0_SET_COMPARE(M25);
  SYSKEY = 0;                       // ensure OSCCON is locked
  SYSKEY = 0xAA996655;              // unlock sequence
  SYSKEY = 0x556699AA;
  CFGCONbits.IOLOCK = 0;            // allow write
  SDI2R = 0b0100;                   // PB2
  RPB1R = 0b0100;                   // SDO2
  CFGCONbits.IOLOCK = 1;            // forbidden write
  SYSKEY = 0;                       // relock
  ANSELBCLR = 0xf;
  TRISBCLR = 0x200;                 // RB9 (LED)
  CNPUB = 0x100;                    // RB8
  CNPUC = 0x200;                    // RC9
  IPC0bits.CTIP = 1;
  IEC0bits.CTIE = 1;
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

bool wait(unsigned i) {
    unsigned u = tick;
    i <<= 2;
    for (loop(u); i--; u = tick) while (u == tick) loop(u);
    return true;
}

void wait_M25(void) {
  unsigned u = tick;
  while (u == tick);
}

extern "C"
void __attribute__((interrupt(ipl1soft), vector(_CORE_TIMER_VECTOR), nomips16)) rtcISR(void) {
  _CP0_SET_COMPARE(_CP0_GET_COMPARE() + M25);
  tick++;
  IFS0bits.CTIF = 0;
}

void toggle(void) { LATBINV = 0x200; }
void led_on(void) { LATBSET = 0x200; }
void led_off(void) { LATBCLR = 0x200; }

void button(void) {
  static int b = 0x100;
  static int c = 0x200;
  if (b != (PORTB & 0x100)) button_changed(b ^= 0x100);
  if (c != (PORTC & 0x200)) button_changed(c ^= 0x200);
}

