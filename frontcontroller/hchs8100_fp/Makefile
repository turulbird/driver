ifeq ($(KERNELRELEASE),)
	DRIVER_TOPDIR:=$(shell pwd)/..
	include $(DRIVER_TOPDIR)/kernel.make
else
	hchs8100_fp-objs += hchs8100_fp_main.o
	hchs8100_fp-objs += hchs8100_fp_file.o
	hchs8100_fp-objs += hchs8100_fp_procfs.o
	obj-m += hchs8100_fp.o

	ifneq (,$(findstring 2.6.3,$(KERNELVERSION)))
		ccflags-y += $(CCFLAGSY)
	else
		CFLAGS += $(CCFLAGSY)
	endif
endif

