snd-soc-fpga-codec-objs := fpga-codec.o
obj-$(CONFIG_SND_SOC_FPGA_SOUNDCARD)  += snd-soc-fpga-codec.o

# older kernels
dtb-y += fpga-soundcard.dtbo

%.dtbo: %.dts
	dtc -o dtb -o $@ -b 0 -@ $<

targets += $(dtb-y)
always	:= $(dtb-y)
clean-files	:= *.dtbo
