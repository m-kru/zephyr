ccflags-y += -I${ZEPHYR_BASE}/tests/include
ccflags-y += -I${ZEPHYR_BASE}/subsys/net/ip/l2/ieee802154
ccflags-y += -I${ZEPHYR_BASE}/subsys/net/ip/

include $(ZEPHYR_BASE)/tests/Makefile.test

obj-y = ieee802154_test.o \
	ieee802154_fake_driver.o
