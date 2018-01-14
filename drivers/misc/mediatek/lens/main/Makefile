#
# Copyright (C) 2015 MediaTek Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#

#
# Makefile for misc devices that really don't fit anywhere else.
#

MTK_PLATFORM := $(subst ",,$(CONFIG_MTK_PLATFORM))
ARCH_MTK_PROJECT := $(subst ",,$(CONFIG_ARCH_MTK_PROJECT))

MTK_CUSTOM_PATH  := $(srctree)/drivers/misc/mediatek

ifneq ($(CONFIG_ARM64), y)
# For arm32, mach folder
MTK_PLATFORM_PATH_ROOT := $(srctree)/arch/arm/mach-$(MTK_PLATFORM)
MTK_PROJECT_PATH_ROOT := $(srctree)/arch/arm/mach-$(MTK_PLATFORM)/$(ARCH_MTK_PROJECT)
  ifneq ($(filter y,$(CONFIG_ARCH_MT6752) $(CONFIG_ARCH_MT6732) $(CONFIG_ARCH_MT6595) $(CONFIG_ARCH_MT6795) $(CONFIG_ARCH_MT6735) $(CONFIG_ARCH_MT6735M) $(CONFIG_ARCH_MT6753) $(CONFIG_ARCH_MT6580)),)
    # For armv8 32bit
    MTK_PLATFORM_PATH_ROOT := $(srctree)/drivers/misc/mediatek/mach/$(MTK_PLATFORM)
    MTK_PROJECT_PATH_ROOT := $(srctree)/drivers/misc/mediatek/mach/$(MTK_PLATFORM)/$(ARCH_MTK_PROJECT)
  endif
else
# For arm64, no mach-xxx folder
MTK_PLATFORM_PATH_ROOT := $(srctree)/drivers/misc/mediatek/mach/$(MTK_PLATFORM)
MTK_PROJECT_PATH_ROOT := $(srctree)/drivers/misc/mediatek/mach/$(MTK_PLATFORM)/$(ARCH_MTK_PROJECT)
endif



subdir-ccflags-y += -I$(MTK_CUSTOM_PATH)/lens/main/inc
subdir-ccflags-$(CONFIG_MTK_LENS_LC898122AF_SUPPORT) += -I$(MTK_CUSTOM_PATH)/lens/main/common/lc898122af
subdir-ccflags-$(CONFIG_MTK_LENS_LC898212XDAF_SUPPORT) += -I$(MTK_CUSTOM_PATH)/lens/main/common/lc898212xdaf/inc
subdir-ccflags-$(CONFIG_MTK_LENS_LC898212XDAF_SUPPORT) += -I$(MTK_CUSTOM_PATH)/lens/main/common/lc898212xdaf_f/inc
subdir-ccflags-$(CONFIG_MTK_LENS_BU63165AF_SUPPORT) += -I$(MTK_CUSTOM_PATH)/lens/main/common/bu63165af/inc

ifeq ($(CONFIG_MTK_LENS_USE_MODULE), y)

MAIN_CFILES += main_lens.c

ifeq ($(CONFIG_MTK_LENS_AD5820AF_SUPPORT),y)
MAIN_CFILES += common/ad5820af/AD5820AF.c
endif

ifeq ($(CONFIG_MTK_LENS_DW9714AF_SUPPORT),y)
MAIN_CFILES += common/dw9714af/DW9714AF.c
endif
ifeq ($(CONFIG_MTK_LENS_LC898212AF_SUPPORT),y)
MAIN_CFILES += common/lc898212af/LC898212AF.c
endif

obj-m := mainaf.o
mainaf-objs := $(MAIN_CFILES:.c=.o)

else

obj-y      += main_lens.o
obj-$(CONFIG_MTK_LENS_GC8024AF_SUPPORT)      += common/gc8024af/
obj-$(CONFIG_MTK_LENS_GC5005AF_SUPPORT)      += common/gc5005af/
obj-$(CONFIG_MTK_LENS_GC5024AF_SUPPORT)      += common/gc5024af/
obj-$(CONFIG_MTK_LENS_HM5040AF_SUPPORT)      += common/hm5040af/
obj-$(CONFIG_MTK_LENS_IMX179AF_SUPPORT)      += common/imx179af/
obj-$(CONFIG_MTK_LENS_S5K3H7AF_SUPPORT)      += common/s5k3h7af/
obj-$(CONFIG_MTK_LENS_S5K3H7YXAF_SUPPORT)      += common/s5k3h7yxaf/
obj-$(CONFIG_MTK_LENS_OV8865AF_SUPPORT)      += common/ov8865af/
obj-$(CONFIG_MTK_LENS_FM50AF_SUPPORT)      += common/fm50af/
obj-$(CONFIG_MTK_LENS_BU6424AF_SUPPORT)    += common/bu6424af/
obj-$(CONFIG_MTK_LENS_BU6429AF_SUPPORT)    += common/bu6429af/
obj-$(CONFIG_MTK_LENS_BU63165AF_SUPPORT)  += common/bu63165af/
obj-$(CONFIG_MTK_LENS_DW9714AF_SUPPORT)    += common/dw9714af/
obj-$(CONFIG_MTK_LENS_DW9763AF_SUPPORT)    += common/dw9763af/
obj-$(CONFIG_MTK_LENS_DW9814AF_SUPPORT)    += common/dw9814af/
obj-$(CONFIG_MTK_LENS_DW9718AF_SUPPORT)    += common/dw9718af/
obj-$(CONFIG_MTK_LENS_DW9719TAF_SUPPORT)    += common/dw9719taf/
obj-$(CONFIG_MTK_LENS_LC898212AF_SUPPORT)  += common/lc898212af/
obj-$(CONFIG_MTK_LENS_LC898212XDAF_SUPPORT)  += common/lc898212xdaf/
obj-$(CONFIG_MTK_LENS_LC898214AF_SUPPORT)  += common/lc898214af/
obj-$(CONFIG_MTK_LENS_LC898122AF_SUPPORT)  += common/lc898122af/
obj-$(CONFIG_MTK_LENS_AD5820AF_SUPPORT)    += common/ad5820af/
obj-$(CONFIG_MTK_LENS_WV511AAF_SUPPORT)    += common/wv511aaf/
obj-$(CONFIG_MTK_LENS_AK7371AF_SUPPORT)    += common/ak7371af/
obj-$(CONFIG_MTK_LENS_MN34172AF_SUPPORT)      += common/mn34172af/
obj-$(CONFIG_MTK_LENS_MN045AF_SUPPORT)      += common/mn045af/
obj-$(CONFIG_MTK_LENS_S5K3L2AF_SUPPORT)      += common/s5k3l2af/
obj-$(CONFIG_MTK_LENS_SP5506AF_SUPPORT)    += common/sp5506af/
obj-$(CONFIG_MTK_LENS_IMX237AF_SUPPORT)    += common/imx237af/
obj-$(CONFIG_MTK_LENS_IMX214AF_SUPPORT)    += common/imx214af/
obj-$(CONFIG_MTK_LENS_IMX219AF_SUPPORT)    += common/imx219af/
obj-$(CONFIG_MTK_LENS_SP8407AF_SUPPORT)    += common/sp8407af/
obj-$(CONFIG_MTK_LENS_OV8858AF_SUPPORT)      += common/ov8858af/
obj-$(CONFIG_MTK_LENS_HI545AF_SUPPORT)    += common/hi545af/
obj-$(CONFIG_MTK_LENS_IMX149AF_SUPPORT)    += common/imx149af/
obj-$(CONFIG_MTK_LENS_S5K3L8AF_SUPPORT)    	+= common/s5k3l8af/
obj-$(CONFIG_MTK_LENS_OV5693AF_SUPPORT)    	+= common/ov5693af/
endif
