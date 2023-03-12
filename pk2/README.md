# Pickit 2 clone, 3.3 volts only
![](https://github.com/dgtie/dgtie.github.io/blob/main/pk2.png?raw=true)
- programmer application: pic32prog
- development board: [STICK250](https://lamsworkshop.blogspot.com/2023/01/stick250-pic32mx250f128d-experiment.html)

![](https://blogger.googleusercontent.com/img/b/R29vZ2xl/AVvXsEhVIuT3C0-lhgrR2kOnv1O7wgonlwH7WAipyoY_wCIvZt8i6TwSRL1mn-vF0JERCa3GLuwmxFus5kegwttRMdlRZcjDRPzHh-tDQjY_ykTlgmJQYV7XPG3O1YpYbtYN4wfl3-9EZdXS3CbG3dvzMZhsNn1dgEx2FlSlJAGuRDgxKI-Xad8t22gcGvYz/s320/pickit2.png)
# udev rule
```
# Pickit 2
ATTRS{idVendor}=="04d8", ATTRS{idProduct}=="0033", MODE="0666", ENV{ID_MM_DEVICE_IGNORE}="1", ENV{ID_MM_PORT_IGNORE}="1"
```

