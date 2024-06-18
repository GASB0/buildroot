################################################################################
#
# Custom uart module
#
################################################################################

CUSTOM_UART_VERSION = 0.0.1
CUSTOM_UART_SITE = package/custom-uart
CUSTOM_UART_SITE_METHOD = local
CUSTOM_UART_DEPENDENCIES =
CUSTOM_UART_INSTALL_IMAGES = YES

# Kernel module section
CUSTOM_UART_MODULE_SUBDIRS = serial
CUSTOM_UART_MODULE_MAKE_OPTS = \
        INSTALL_MOD_DIR=kernel/drivers/misc \
        CONFIG_CUSTOM_UART=m \
        CONFIG_CUSTOM_UART_DEBUG=y

define CUSTOM_UART_INSTALL_OVERLAYS
        $(INSTALL) -D -m 0644 $(@D)/serial/custom-uart.dtbo $(BINARIES_DIR)/rpi-firmware/overlays/
endef

define CUSTOM_UART_INSTALL_IMAGES_CMDS
        $(CUSTOM_UART_INSTALL_OVERLAYS)
endef

$(eval $(kernel-module))
$(eval $(generic-package))
