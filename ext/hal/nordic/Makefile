ifdef CONFIG_HAS_NORDIC_MDK
ZEPHYRINCLUDE += -I$(srctree)/ext/hal/nordic/mdk
endif

ifdef CONFIG_HAS_NORDIC_HAL
ZEPHYRINCLUDE += -I$(srctree)/ext/hal/nordic/hal
endif

ifdef CONFIG_HAS_NORDIC_DRIVERS
ZEPHYRINCLUDE += -I$(srctree)/ext/hal/nordic/drivers
endif

ifdef CONFIG_HAS_NRFX
ZEPHYRINCLUDE += -I$(srctree)/ext/hal/nordic/nrfx
ZEPHYRINCLUDE += -I$(srctree)/ext/hal/nordic/nrfx/drivers/include
ZEPHYRINCLUDE += -I$(srctree)/ext/hal/nordic
endif

obj- += dummy.o
obj-$(CONFIG_PRS_NRFX) += nrfx/drivers/src/prs/nrfx_prs.o
obj-$(CONFIG_PWM_NRFX) += nrfx/drivers/src/nrfx_pwm.o
obj-$(CONFIG_RTC_NRFX) += nrfx/drivers/src/nrfx_rtc.o
obj-$(CONFIG_TWI_NRFX) += nrfx/drivers/src/nrfx_twi.o
obj-$(CONFIG_TWIM_NRFX) += nrfx/drivers/src/nrfx_twim.o
