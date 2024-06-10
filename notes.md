# Getting the simple-card module to work
This is the device tree file used as reference in order to get the I2S audio out:
- https://github.com/raspberrypi/linux/issues/2547

Once a linux image with the appropriate overlay has been built, the next step is to load the 
following modules:
```bash
# Getting the fpga audio card to work
modprobe snd-soc-simple-card
modprobe snd-soc-bcm2835-i2s
modprobe regmap-i2c
insmod ./snd-soc-fpga-codec.ko

rmmod snd_soc_simple_card
rmmod snd_soc_fpga_codec

modprobe snd_soc_simple_card
insmod ./snd-soc-fpga-codec.ko
```

Starting serial debugging:
```bash
# On target machine
modprobe snd-soc-simple-card
modprobe snd-soc-bcm2835-i2s
modprobe regmap-i2c
insmod ./snd-soc-fpga-codec.ko

echo ttyAMA0 > /sys/module/kgdboc/parameters/kgdboc
echo g > /proc/sysrq-trigger

# On the host machine gdb
stty -F /dev/ttyUSB0 115200
./output/host/bin/arm-buildroot-linux-gnueabihf-gdb ./output/build/linux-custom/vmlinux -iex "set auto-load safe-path /" -tui
set logging enabled on
set serial baud 115200
target remote /dev/ttyUSB0
lx-symbols /home/gabriel/Documents/Projects/buildroot/output/build/fpga-soundcard-0.0.1/module-fpga-soundcard/
break fpga-codec.c:111

continue

# On target mahcine
rmmod snd_soc_simple_card
rmmod snd_soc_fpga_codec

modprobe snd-soc-simple-card
insmod ./snd-soc-fpga-codec.ko
```

Debugging loadble kernel modules with with kgdb:
https://stackoverflow.com/questions/6260927/module-debugging-through-kgdb

Debugging using gbserver
```bash
./output/host/bin/arm-buildroot-linux-gnueabihf-gdb ./output/build/fpga-soundcard-0.0.1/fpga-soundcard-simple-client/simple_filter_client -tui -ex "target remote | ssh -T root@buildroot.local gdbserver - /root/simple_filter_client"
```
# People with similar issues to yours
https://community.nxp.com/t5/i-MX-Processors/iMX6DL-with-SGTL5000-codec-ALSA-doesn-t-find-soundcard/m-p/272887
Logs with the I2C function:
```dmesg
[  114.401381] calling  alsa_sound_init+0x0/0xa4 [snd] @ 189
[  114.401588] initcall alsa_sound_init+0x0/0xa4 [snd] returned 0 after 59 usecs
[  114.405480] calling  alsa_timer_init+0x0/0x1000 [snd_timer] @ 189
[  114.405769] initcall alsa_timer_init+0x0/0x1000 [snd_timer] returned 0 after 219 usecs
[  114.416074] calling  alsa_pcm_init+0x0/0x1000 [snd_pcm] @ 189
[  114.416228] initcall alsa_pcm_init+0x0/0x1000 [snd_pcm] returned 0 after 11 usecs
[  114.439139] calling  snd_soc_init+0x0/0xb0 [snd_soc_core] @ 189
[  114.439783] probe of snd-soc-dummy returned 0 after 76 usecs
[  114.439949] initcall snd_soc_init+0x0/0xb0 [snd_soc_core] returned 0 after 548 usecs
[  114.445474] calling  asoc_simple_card_init+0x0/0x1000 [snd_soc_simple_card] @ 189
[  114.446082] probe of soc:sound returned 517 after 418 usecs
[  114.446225] initcall asoc_simple_card_init+0x0/0x1000 [snd_soc_simple_card] returned 0 after 701 usecs
[  114.461591] calling  bcm2835_i2s_driver_init+0x0/0x1000 [snd_soc_bcm2835_i2s] @ 190
[  114.463713] probe of fe203000.i2s returned 0 after 1813 usecs
[  114.464415] initcall bcm2835_i2s_driver_init+0x0/0x1000 [snd_soc_bcm2835_i2s] returned 0 after 2657 usecs
[  114.465014] probe of soc:sound returned 517 after 742 usecs
[  114.946555] snd_soc_fpga_codec: loading out-of-tree module taints kernel.
[  114.955428] calling  fpga_codec_driver_init+0x0/0x1000 [snd_soc_fpga_codec] @ 191
[  114.955735] probe of 1-0013 returned 0 after 175 usecs
[  114.956357] probe of soc:sound returned 517 after 445 usecs
[  114.958880] initcall fpga_codec_driver_init+0x0/0x1000 [snd_soc_fpga_codec] returned 0 after 3356 usecs
```

Logs without the I2C function:
```dmesg
[   94.860232] calling  alsa_sound_init+0x0/0xa4 [snd] @ 192
[   94.860439] initcall alsa_sound_init+0x0/0xa4 [snd] returned 0 after 58 usecs
[   94.864357] calling  alsa_timer_init+0x0/0x1000 [snd_timer] @ 192
[   94.864633] initcall alsa_timer_init+0x0/0x1000 [snd_timer] returned 0 after 205 usecs
[   94.874967] calling  alsa_pcm_init+0x0/0x1000 [snd_pcm] @ 192
[   94.875121] initcall alsa_pcm_init+0x0/0x1000 [snd_pcm] returned 0 after 11 usecs
[   94.898044] calling  snd_soc_init+0x0/0xb0 [snd_soc_core] @ 192
[   94.898694] probe of snd-soc-dummy returned 0 after 71 usecs
[   94.898895] initcall snd_soc_init+0x0/0xb0 [snd_soc_core] returned 0 after 588 usecs
[   94.904406] calling  asoc_simple_card_init+0x0/0x1000 [snd_soc_simple_card] @ 192
[   94.905002] probe of soc:sound returned 517 after 415 usecs
[   94.905145] initcall asoc_simple_card_init+0x0/0x1000 [snd_soc_simple_card] returned 0 after 690 usecs
[   95.256698] calling  bcm2835_i2s_driver_init+0x0/0x1000 [snd_soc_bcm2835_i2s] @ 193
[   95.257415] probe of fe203000.i2s returned 0 after 595 usecs
[   95.257652] initcall bcm2835_i2s_driver_init+0x0/0x1000 [snd_soc_bcm2835_i2s] returned 0 after 900 usecs
[   95.258076] probe of soc:sound returned 517 after 462 usecs
[  105.075334] snd_soc_fpga_codec: loading out-of-tree module taints kernel.
[  105.083940] calling  bare_codec_driver_init+0x0/0x1000 [snd_soc_fpga_codec] @ 195
[  105.084227] probe of spdif-transmitter returned 0 after 150 usecs
[  105.084459] initcall bare_codec_driver_init+0x0/0x1000 [snd_soc_fpga_codec] returned 0 after 430 usecs
[  105.087837] probe of soc:sound returned 0 after 3419 usecs
```
