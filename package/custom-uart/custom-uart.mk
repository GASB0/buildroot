################################################################################
#
# Custom uart module
#
################################################################################

CUSTOM_UART_VERSION = 0.0.1
CUSTOM_UART_SITE = package/custom-uart
CUSTOM_UART_SITE_METHOD = local
CUSTOM_UART_DEPENDENCIES =

# Kernel module section
CUSTOM_UART_MODULE_SUBDIRS = serial
CUSTOM_UART_MODULE_MAKE_OPTS = \
        INSTALL_MOD_DIR=kernel/drivers/misc \
        CONFIG_CUSTOM_UART=m \
        CONFIG_CUSTOM_UART_DEBUG=y

define CUSTOM_UART_INSTALL_MOD
        # $(INSTALL) -D -m 644 $(@D)/$(CUSTOM_UART_MODULE_SUBDIRS)/fpga-soundcard.dtbo $(BUILD_DIR)/rpi-firmware-$(RPI_FIRMWARE_VERSION)/boot/overlays/

        # scp -rpO $(BUILD_DIR)/fpga-soundcard-$(CUSTOM_UART_VERSION)/module-fpga-soundcard/snd-soc-fpga-codec.ko root@buildroot.local:/root/
endef
CUSTOM_UART_POST_INSTALL_TARGET_HOOKS += CUSTOM_UART_INSTALL_MOD

$(eval $(kernel-module))
$(eval $(generic-package))
