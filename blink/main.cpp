/*
    a simple demo program
    it only blinks LED

    to work with bootloader:
    startup code is moved to 0xBFC00400 (see app.ld)
    bootloader occupies 0x1D01C000 - 0x1D01EFFF
    application should be smaller than 0x1C000 (112k)
    application should not change configuration registers

         DEVCFG3 = 0x8FFFFFFF
         DEVCFG2 = 0xFFF979D9
         DEVCFG1 = 0xFF74CDDB
         DEVCFG0 = 0x7FFFFFFB
*/


void init(void), toggle_LED(void);
bool wait(unsigned);    // always return true

int main(void) {
  init();
  while (wait(0));      // call wait() to transfer control to poll()
}

void poll(unsigned t) {
  if (!(t & 1023)) toggle_LED();        // toggle every 1024 ms
}
