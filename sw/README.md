Udev rules
----------

$ cat /etc/udev/rules.d/80-knixx.rules
# see: http://linux-tips.com/t/prevent-modem-manager-to-capture-usb-serial-devices/284ATTRS{idVendor}=="1209" ATTRS{idProduct}=="a7ea", ENV{ID_MM_DEVICE_IGNORE}="1"
SUBSYSTEMS=="usb", ACTION=="add", ATTRS{idVendor}=="1209", ATTRS{idProduct}=="a7ea", GROUP="knixx", MODE="0666", SYMLINK+="knixx/console" 

$ sudo udevadm console --reload-rules

Dual CDC
--------

http://akb77.com/g/stm32/stm32f103-cdc/	
http://akb77.com/g/stm32/stm32f103-cdc/	

Debugging it : http://elinux.org/images/1/17/USB_Debugging_and_Profiling_Techniques.pdf

GDB
---

$arm-none-eabi-gdb main.elf

(gdb) target remote :4242

(gdb) load

(gdb) c

Serial interface
----------------
At the moment the following:

* hello
* help
* pry
* peek
* poke

