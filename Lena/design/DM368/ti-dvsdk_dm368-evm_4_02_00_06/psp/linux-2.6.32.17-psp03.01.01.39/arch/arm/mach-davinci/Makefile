#
# Makefile for the linux kernel.
#
#

# Common objects
obj-y 			:= time.o clock.o serial.o io.o psc.o \
			   gpio.o dma.o usb.o common.o sram.o aemif.o

obj-$(CONFIG_DAVINCI_MUX)		+= mux.o
obj-$(CONFIG_PCI)			+= pci-generic.o

# Chip specific

obj-$(CONFIG_ARCH_DAVINCI_DM365)	+= dm365.o devices.o


obj-$(CONFIG_AINTC)			+= irq.o
obj-$(CONFIG_CP_INTC)			+= cp_intc.o

# Board specific

obj-$(CONFIG_MACH_DAVINCI_DM365_EVM)	+= board-dm365-evm.o

