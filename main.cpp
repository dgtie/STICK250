void init(void), USBDeviceInit(void), ProcessIO(void);
void toggle_LED(void), set_LED(void), clear_LED(void);
bool wait(unsigned);    // always return true

int main(void) {
  init();
  USBDeviceInit();
  while (wait(0)) ProcessIO();      // call wait() to transfer control to poll()
}

void poll(unsigned t) {
//  if (!(t & 1023)) toggle_LED();        // toggle every 1024 ms
}
