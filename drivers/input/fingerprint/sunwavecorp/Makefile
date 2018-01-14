
ccflags-y += -I$(srctree)/drivers/spi/mediatek/$(MTK_PLATFORM)

ccflags-y += -Wno-error

MODFLAGS       =$(MODFLAGS)  -DMODULE -fno-pic

sunwave-objs := sunwavecorp.o platform.o misce.o mt6797_dts.o mt6797_dts_plat.o

#ifdef CONFIG_SUNWAVECORP_MT6797
sunwave-objs +=  mt6797_dts.o
obj-y= sunwave.o mt6797_dts_plat.o
#endif

