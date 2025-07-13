KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)
SRC  := $(PWD)/src
BUILD := $(PWD)/build

EXTRA_CFLAGS += -I$(SRC)
TEST_DIR := ${PWD}/tests/test_ioctl
TEST_SRC := $(TEST_DIR)/test_ioctl.c
TEST_BIN := $(PWD)/tests/test_ioctl.o

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

.PHONY: test

test:
	$(CC) -Wall -Wextra -I./src -o $(TEST_BIN) $(TEST_SRC)
