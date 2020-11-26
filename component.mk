ARDUINO_LIBRARIES :=  OneWire ArduinoJson5

SPI_SIZE		?= 4M

RBOOT_ENABLED		?= 1
RBOOT_ROM1_ADDR 	?= 0x102000

#RBOOT_BIG_FLASH		?= 1
#RBOOT_TWO_ROMS		?= 0
#RBOOT_SPIFFS_0		?= 0x100000
#RBOOT_SPIFFS_1		?= 0x300000
RBOOT_RTC_ENABLED	= 1

#DISABLE_SPIFFS		?= 1

SPIFF_SIZE		?= 524288

ENABLE_CMD_EXECUTOR	= 0

COMPONENT_SRCDIRS = app lib/application
COMPONENT_INCDIRS = include lib/application lib/wsbinconst

WEBPACK_OUT = files/index.*.js*

.PHONY: webpacker
webpacker: 
	$(vecho) "Bundling js with Webpack"
	$(Q) rm -rf $(WEBPACK_OUT)
	$(Q) webpack
