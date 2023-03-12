# Sample User Application
- startup code is moved to 0xBFC00400 (see app.ld)
- code size must be less than 112k bytes
- configuration registers cannot be modified

# Configuration Registers
|Register|Value|
|-|-|
|DEVCFG3|0x0FFFFFFF|
|DEVCFG2|0xFFF979D9|
|DEVCFG1|0xFF74CDDB|
|DEVCFG0|0x7FFFFFFB|

# My Programming Framework for microcontroller
- simple and short
- one main thread to do blocking sequential job
- background tasks run cooperatively with main thread at the same user level
- need one timer to update time-stamp

# How to program in the framework
>bool wait(unsigned time);
 
- only call it from the main thread
- it blocks main thread for the time specified
- background tasks are serviced during the blockage
- wait(0) will check to run background tasks ONCE

>void poll(unsigned timestamp);

- to be implemented by user application
- will be called once when timestamp changes
- never call wait() from poll() since poll() is called from wait()
- call background tasks from it
- background task must not be blocking and returns quickly

>void loop(unsigned timestamp);

- can be modified by user application to run background tasks
- called **continuously** from wait() until wait() return to main thread
- it calls poll() only when timestamp changes

>unsigned tick;

- timestamp is simply a copy of ***tick***
- it must be incremented in a timer ISR
- update rate is usaully 1 ms hence wait(1000) takes one second
