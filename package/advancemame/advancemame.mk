################################################################################
#
# advancemame
#
################################################################################

ADVANCEMAME_VERSION = v3.9
ADVANCEMAME_SITE = package/advancemame/advancemame-3.9
ADVANCEMAME_SITE_METHOD = local
ADVANCEMAME_LICENSE = GPLv2

ADVANCEMAME_CONF_ENV += LDFLAGS=-"L$(STAGING_DIR)/usr/lib/ -lbcm_host -lvcos -lvchiq_arm -lvchostif" CFLAGS="-w -mcpu=arm1176jzf-s -mfloat-abi=hard -mfpu=vfp -mtune=arm1176jzf-s -O3"

ADVANCEMAME_CONF_OPTS += \
	--with-enu=mame \
        --prefix=$(TARGET_DIR)/usr \
        --exec-prefix=$(TARGET_DIR)/usr \
        --prefix=$(TARGET_DIR)/usr \
        --disable-oss \
        --disable-sdl \
        --disable-sdl2 \
        --enable-alsa \
        --enable-vc \
	--disable-ncurses \
	--with-vc-prefix=$(TARGET_DIR)/usr/lib


ADVANCEMAME_DEPENDENCIES = \
	alsa-lib \
	rpi-userland

define ADVANCEMAME_INSTALL_TARGET_CMDS
    cp -a $(@D)/obj/mame/linux/blend/advmame $(TARGET_DIR)/usr/bin
endef

$(eval $(autotools-package))

