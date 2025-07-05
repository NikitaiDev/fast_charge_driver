KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)
SRC  := $(PWD)/src
BUILD := $(PWD)/build

EXTRA_CFLAGS += -I$(SRC)

all:
	$(MAKE) -C $(KDIR) M=$(SRC) modules
	mkdir -p $(BUILD)
	cp $(SRC)/fast_charge.ko $(BUILD)/

clean:
	$(MAKE) -C $(KDIR) M=$(SRC) clean
	rm -rf $(BUILD)

insmod: all
	sudo insmod $(BUILD)/fast_charge.ko

rmmod:
	sudo rmmod fast_charge

