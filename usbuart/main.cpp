void init(void), wait(unsigned), toggle(void), USBDeviceInit(void);

int main(void) {
    init();
    USBDeviceInit();
    while (1) wait(0);
}

void poll(unsigned t) {
    if (t & 511) return;
    toggle();
}
