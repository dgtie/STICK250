#include <xc.h>                    // Device specific definitions

#define MS 20000

void init(void) {
    __builtin_disable_interrupts();
    _CP0_SET_COMPARE(MS);
    IPC0bits.CTIP = 1;
    IEC0bits.CTIE = 1;
    INTCONSET = _INTCON_MVEC_MASK;
    __builtin_enable_interrupts();
}

void poll(unsigned);

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
    for (loop(u); i--; u = tick) while (u == tick) loop(u);
    return true;
}

extern "C"
void __attribute__((interrupt(ipl1soft), vector(_CORE_TIMER_VECTOR), nomips16))
ctISR(void) {
    _CP0_SET_COMPARE(_CP0_GET_COMPARE() + MS);
    tick++;
    IFS0bits.CTIF = 0;
}

unsigned getTimeMilli(void) { return tick; }
