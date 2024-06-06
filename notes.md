# Getting the simple-card module to work
This is the device tree file used as reference in order to get the I2S audio out:
- https://github.com/raspberrypi/linux/issues/2547

Once a linux image with the appropriate overlay has been built, the next step is to load the 
following modules:
```bash
# Getting the fpga audio card to work
modprobe snd-soc-simple-card
modprobe snd-soc-bcm2835-i2s
insmod ./snd-soc-fpga-codec.ko

rmmod snd_soc_simple_card
rmmod snd_soc_fpga_codec

insmod ./snd-soc-fpga-codec.ko
```

Starting serial debugging:
```bash
# On target machine
modprobe snd-soc-simple-card
modprobe snd-soc-bcm2835-i2s
insmod ./snd-soc-fpga-codec.ko

echo ttyAMA0 > /sys/module/kgdboc/parameters/kgdboc
echo g > /proc/sysrq-trigger

# On the host machine gdb
./output/host/bin/arm-buildroot-linux-gnueabihf-gdb ./output/build/linux-custom/vmlinux -iex "set auto-load safe-path /" -tui
target remote /dev/ttyUSB0
lx-symbols /home/gabriel/Documents/Projects/buildroot/output/build/fpga-soundcard-0.0.1/module-fpga-soundcard/
break fpga-codec.c:45
continue

# On target mahcine
rmmod snd_soc_simple_card
rmmod snd_soc_fpga_codec

modprobe snd-soc-simple-card
insmod ./snd-soc-fpga-codec.ko
```

Debugging loadble kernel modules with with kgdb:
https://stackoverflow.com/questions/6260927/module-debugging-through-kgdb
