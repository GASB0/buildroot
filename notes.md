# Getting the simple-card module to work
In order to make the thing work you should do this:
```bash
modprobe snd-soc-simple-card-utils
modprobe snd-bcm2835

modprobe snd-soc-simple-card
modprobe snd-soc-bcm2835-i2s
modprobe snd-soc-spdif-tx


modprobe snd-soc-rpi-simple-soundcard

modprobe -vs i2c-bcm2708
modprobe -vs snd-soc-bcm2835-i2s
modprobe -vs snd_soc_pcm512x_i2c
modprobe -vs clk_hifiberry_dacpro
modprobe -vs snd_soc_hifiberry_dacplus
```
