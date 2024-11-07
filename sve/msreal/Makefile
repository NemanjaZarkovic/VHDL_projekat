obj-m += surf_driver.o
	
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
# Opcije kompajliranja za isključivanje SSE instrukcija
ccflags-y := -msse -msse2 -mmmx -mfpmath=sse

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean


