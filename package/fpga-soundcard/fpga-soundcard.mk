################################################################################
#
# FPGA soundcard module overlay
#
################################################################################

FPGA_SOUNDCARD_VERSION = 0.0.1
FPGA_SOUNDCARD_SITE = package/fpga-soundcard
FPGA_SOUNDCARD_SITE_METHOD = local
FPGA_SOUNDCARD_DEPENDENCIES =

# Kernel module section
FPGA_SOUNDCARD_MODULE_SUBDIRS = module-fpga-soundcard
FPGA_SOUNDCARD_MODULE_MAKE_OPTS = \
	INSTALL_MOD_DIR=kernel/sound/soc/codecs \
	CONFIG_SND_SOC_FPGA_SOUNDCARD=m \
	CONFIG_SND_SOC_FPGA_SOUNDCARD_DEBUG=y

define FPGA_SOUNDCARD_INSTALL_MOD
	$(INSTALL) -D -m 644 $(@D)/$(FPGA_SOUNDCARD_MODULE_SUBDIRS)/fpga-soundcard.dtbo $(BUILD_DIR)/rpi-firmware-$(RPI_FIRMWARE_VERSION)/boot/overlays/

	scp -rpO $(BUILD_DIR)/fpga-soundcard-$(FPGA_SOUNDCARD_VERSION)/module-fpga-soundcard/snd-soc-fpga-codec.ko root@buildroot.local:/root/
	scp -rpO $(BUILD_DIR)/fpga-soundcard-$(FPGA_SOUNDCARD_VERSION)/module-fpga-soundcard/fpga-soundcard.dtbo root@buildroot.local:/mnt/overlays/
endef
FPGA_SOUNDCARD_POST_INSTALL_TARGET_HOOKS += FPGA_SOUNDCARD_INSTALL_MOD

# Client program section
FPGA_SOUNDCARD_MAKE_OPTS = \
        CFLAGS="$(TARGET_CFLAGS) $(FPGA_SOUNDCARD_BUILD_OPTS)" \
        PROJECT_DIR="$(@D)/fpga-soundcard-simple-client/"

define FPGA_SOUNDCARD_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/fpga-soundcard-simple-client/ CC="$(TARGET_CC)"
endef

define FPGA_SOUNDCARD_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/fpga-soundcard-simple-client/simple_filter_client $(TARGET_DIR)/usr/bin/
	scp -rpO $(@D)/fpga-soundcard-simple-client/simple_filter_client root@buildroot.local:/root/
endef

$(eval $(kernel-module))
$(eval $(generic-package))
