LD_FILE = boards/samd21x18-bootloader-external-flash-crystalless.ld
USB_VID = 0x239A
USB_PID = 0x8019

FLASH_IMPL = spi_flash.c

CHIP_VARIANT = SAMD21G18A
CHIP_FAMILY = samd21

# Include these Python libraries in firmware.
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_NeoPixel
