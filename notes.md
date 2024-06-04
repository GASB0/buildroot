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

# Removing the fpga card
rmmod snd_soc_simple_card
rmmod snd_soc_simple_card_utils
rmmod snd_soc_bcm2835_i2s
rmmod snd_soc_fpga_codec
rmmod snd_soc_core
```

Starting serial debugging:
```bash
# On the host machine gdb
./output/host/bin/arm-buildroot-linux-gnueabihf-gdb ./output/build/linux-custom/vmlinux
set serial baud 115200

# On target machine
MODULE_NAME=snd_soc_fpga_codec
MODULE_FILE=$(modinfo $MODULE_NAME| awk '/filename/{print $2}')
DIR="/sys/module/${MODULE_NAME}/sections/"
# echo add-symbol-file $MODULE_FILE -s $(cat "$DIR/.text") -s .bss $(cat "$DIR/.bss") -s .data $(cat "$DIR/.data") # Run his on host
echo add-symbol-file $MODULE_FILE -s .data $(cat "$DIR/.data") # Run his on host

# On the host machine gdb
target remote /dev/ttyUSB0

echo ttyAMA0 > /sys/module/kgdboc/parameters/kgdboc
echo g > /proc/sysrq-trigger
```

Debugging loadble kernel modules with with kgdb:
https://stackoverflow.com/questions/6260927/module-debugging-through-kgdb
