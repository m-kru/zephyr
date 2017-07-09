ifdef CONFIG_HAS_NRFX
ZEPHYRINCLUDE += -I$(srctree)/ext/hal/nordic/nrfx/mdk
ZEPHYRINCLUDE += -I$(srctree)/ext/hal/nordic/nrfx/hal
ZEPHYRINCLUDE += -I$(srctree)/ext/hal/nordic/nrfx
ZEPHYRINCLUDE += -I$(srctree)/ext/hal/nordic/nrfx/drivers/include
ZEPHYRINCLUDE += -I$(srctree)/ext/hal/nordic
endif

ifdef CONFIG_HAS_NORDIC_DRIVERS
ZEPHYRINCLUDE += -I$(srctree)/ext/hal/nordic/drivers
endif

obj- += dummy.o
obj-$(CONFIG_NRFX_PRS) += nrfx/drivers/src/prs/nrfx_prs.o
obj-$(CONFIG_NRFX_ADC) += nrfx/drivers/src/nrfx_adc.o
obj-$(CONFIG_NRFX_PWM) += nrfx/drivers/src/nrfx_pwm.o
obj-$(CONFIG_NRFX_RTC) += nrfx/drivers/src/nrfx_rtc.o
obj-$(CONFIG_NRFX_SAADC) += nrfx/drivers/src/nrfx_saadc.o
obj-$(CONFIG_NRFX_SPI) += nrfx/drivers/src/nrfx_spi.o
obj-$(CONFIG_NRFX_SPIM) += nrfx/drivers/src/nrfx_spim.o
obj-$(CONFIG_NRFX_TWI) += nrfx/drivers/src/nrfx_twi.o
obj-$(CONFIG_NRFX_TWIM) += nrfx/drivers/src/nrfx_twim.o
obj-$(CONFIG_NRFX_UART) += nrfx/drivers/src/nrfx_uart.o
obj-$(CONFIG_NRFX_UARTE) += nrfx/drivers/src/nrfx_uarte.o
obj-$(CONFIG_NRFX_WDT) += nrfx/drivers/src/nrfx_wdt.o
