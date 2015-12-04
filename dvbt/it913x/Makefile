#TREE_ROOT?=$(DRIVER_TOPDIR)/
#CCFLAGSY += -I$(TREE_ROOT)/dvbt/it913x/dvb-usb
#CCFLAGSY += -I$(TREE_ROOT)/dvbt/it913x/dvb-core
#CCFLAGSY += -I$(TREE_ROOT)/include
CCFLAGSY += -DDEBUG_DVB

ifneq (,$(findstring 2.6.3,$(KERNELVERSION)))
ccflags-y += $(CCFLAGSY)
else
CFLAGS += $(CCFLAGSY)
endif

dvb-it913x-objs := it913x.o it913x-fe.o dvb-usb-stub.o
obj-m := dvb-it913x.o

#dvb-usb-objs := dvb-usb/dvb-usb-firmware.o
#dvb-usb-objs += dvb-usb/dvb-usb-init.o
#dvb-usb-objs += dvb-usb/dvb-usb-urb.o
#dvb-usb-objs += dvb-usb/dvb-usb-i2c.o
#dvb-usb-objs += dvb-usb/dvb-usb-dvb.o
#dvb-usb-objs += dvb-usb/dvb-usb-remote.o
#dvb-usb-objs += dvb-usb/usb-urb.o
#obj-m := dvb-usb.o

#dvb-core-objs := dvb-core/dvbdev.o
#dvb-core-objs += dvb-core/dmxdev.o
#dvb-core-objs += dvb-core/dvb_demux.o
#dvb-core-objs += dvb-core/dvb_filter.o
#dvb-core-objs += dvb-core/dvb_frontend.o
#dvb-core-objs += dvb-core/dvb_net.o
#dvb-core-objs += dvb-core/dvb_ringbuffer.o
#dvb-core-objs += dvb-core/dvb_math.o
#obj-m := dvb-core.o
 
EXTRA_CFLAGS += -DLINUX -DCONFIG_IT913X_USB
#EXTRA_CFLAGS += -Idrivers/media/dvb/dvb-core
#EXTRA_CFLAGS += -Idrivers/media/dvb/dvb-usb
#EXTRA_CFLAGS += -Idrivers/media/dvb/frontends
EXTRA_CFLAGS += -Idrivers/usb/core

