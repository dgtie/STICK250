void init(void), toggle_LED(void), USBDeviceInit(void);
bool wait(unsigned);

int main(void) {
    init();
    USBDeviceInit();
    while (wait(0));
}

void read_switch(void);
void poll(unsigned t) { if (!(t & 15)) read_switch(); }

void on_switch_change(bool b) { if (!b) toggle_LED(); }
