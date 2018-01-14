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
# Makefile for misc devices that really don't fit anywhere else.
#
ifeq ($(CONFIG_OPEN_MAIN_FAKE_FLASHLIGHT),y)
obj-y				+= main_fake_leds_strobe.o
else
obj-y				+= leds_strobe.o
endif

