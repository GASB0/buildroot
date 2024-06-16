ccflags-$(CONFIG_CUSTOM_UART_DEBUG) := -g3
obj-$(CONFIG_CUSTOM_UART)  += custom-uart.o
custom-uart-y := serial.o

# older kernels
# dtb-y += fpga-soundcard.dtbo

# %.dtbo: %.dts
#         dtc -o dtb -o $@ -b 0 -@ $<

# targets += $(dtb-y)
# always  := $(dtb-y)
# clean-files     := *.dtbo
