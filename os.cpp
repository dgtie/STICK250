#include <xc.h>

const int devcfg3 __attribute__((section(".config_BFC00BF0"), used)) = 0x0FFFFFFF;
const int devcfg2 __attribute__((section(".config_BFC00BF4"), used)) = 0xFFF979D9;
const int devcfg1 __attribute__((section(".config_BFC00BF8"), used)) = 0xFF74CDDB;
const int devcfg0 __attribute__((section(".config_BFC00BFC"), used)) = 0x7FFFFFFB;

#define MS 20000        // 1 ms

void init(void) {
  TRISBCLR = 1 << 9;            // clear tris -> set B9 as output
  _CP0_SET_COMPARE(MS);         // set core timer interrupt condition
  IPC0bits.CTIP = 1;            // core timer interrupt at lowest priority
  IEC0bits.CTIE = 1;            // enable core timer interrupt
  INTCONSET = _INTCON_MVEC_MASK;
  __builtin_enable_interrupts();        // global interrupt enable
}

void toggle_LED(void) { LATBINV = 1 << 9; }
void set_LED(void) { LATBSET = 1 << 9; }
void clear_LED(void) { LATBCLR = 1 << 9; }

static volatile unsigned tick;

void USB_poll(void);
void poll(unsigned timestamp);  // it will be called when timestamp changes

static void loop(unsigned t) {
  USB_poll();
  static unsigned tick;
  if (tick != t) poll(tick = t);
}

bool wait(unsigned t) {
  unsigned u = tick;
  for (loop(u); t--; u = tick) while (u == tick) loop(u);
  return true;
}

extern "C"
__attribute__((interrupt(ipl1soft), vector(_CORE_TIMER_VECTOR), nomips16))
void ctisr(void) {
  _CP0_SET_COMPARE(_CP0_GET_COMPARE() + MS);    // next interrupt at 1 ms
  tick++;                                       // keep track of time
  IFS0bits.CTIF = 0;                            // clear flag
}

void __attribute__((section (".reset2"), naked)) reset(void) {
  CNPUBSET = 1 << 8;
  if (*(int*)(0xbfc00400) == -1) asm("jal 0xbfc00080");
  while (PR1--) asm("nop");
  if (PORTBbits.RB8) asm("jal 0xbfc00400");
  asm("jal 0xbfc00080");
}

void jump_app(void) {
  U1CONbits.USBEN = 0;
  U1PWRCbits.USBPWR = 0;
  U1ADDR = U1EIR = U1IR = U1EP0 = 0;
  asm("jal 0xbfc00400");
}
