# AN1388 HID Bootloader Programmer Application
![](https://github.com/dgtie/dgtie.github.io/blob/main/prog.png?raw=true)

# Build Prequisite
```
$sudo apt install libhidapi-dev
```

# udev rule
```
# AN1388 HID Bootloader
ATTRS{idVendor}=="04d8", ATTRS{idProduct}=="003c", MODE="0666", ENV{ID_MM_DEVICE_IGNORE}="1", ENV{ID_MM_PORT_IGNORE}="1"
```

# Things Learned
- stdout is line buffered. fflush(stdout) to force printing
