# Sample USB CDC Application
- single virtual com port
- a loopback demostration
- press user button to toggle LED
- buffer size 64 bytes

# USB CDC files
- header files: usb.h usb_config.h uart.h
- usb_descriptors.cpp
- usb_device.cpp
- usb_cdc.cpp
- uart.cpp

# How to program

>void USBDeviceInit(void);
 
- Initialize the cdc device

>bool send_cdc(char *buffer, int length);

- send data to host
- data length must be under 64 (bytes)
- return true if the request is accepted
- the data buffer should not be modified until data are sent. Call send_cdc(0, 0) to check.

>int read_cdc(char* &buffer);

- read data from host
- a pointer to data is passed back in "buffer"
- return number of bytes read

