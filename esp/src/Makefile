# tnx to mamalala
# Changelog
# Changed the variables to include the header file directory
# Added global var for the XTENSA tool root
#
# This make file still needs some work.
#
# Updated for SDK 0.9.2
#
# Output directors to store intermediate compiled files
# relative to the project directory
DATETIME    = $(shell date "+%Y-%b-%d_%H:%M:%S_%Z")
ROOT		= /f/projekti/doma/termostat/espressif
BUILD_BASE	= /f/projekti/doma/termostat/source/esp/builds
FW_BASE		= /f/projekti/doma/termostat/source/esp/builds
COMPILE     = gcc

# Base directory for the compiler
XTENSA_TOOLS_ROOT ?= $(ROOT)/xtensa-lx106-elf/bin

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= $(ROOT)/../source/esp

#Esptool.py path and port
ESPTOOL		?= $(SDK_BASE)/tools/esptool.py
ESPPORT		?= COM3

# name for the target project
TARGET		= app

# which modules (subdirectories) of the project to include in compiling
MODULES		= driver user u8glib user/cpp mqtt
EXTRA_INCDIR    = include $(ROOT)/include/ include/driver include/cpp mqtt/include

# libraries used in this project, mainly provided by the SDK
LIBS		= c gcc hal phy net80211 lwip wpa upgrade main pp json ssl pwm crypto

# compiler flags using during compilation of source files
DEFINITIONS = -DICACHE_FLASH -DBUILD_TIME=\"$(DATETIME)\" -DDEBUG_ON
CFLAGS		= -Os -g -O2 -Wpointer-arith -Wundef -Wl,-EL -fno-inline-functions -ffunction-sections -fdata-sections -nostdlib -mlongcalls -mtext-section-literals -fno-builtin-printf -D__ets__ $(DEFINITIONS)
CXXFLAGS	= $(CFLAGS) -fno-rtti -std=c++11 -fno-exceptions

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static -Wl,--gc-section

# linker script used for the above linkier step
LD_SCRIPT	= eagle.app.v6.new.2048.ld

# various paths from the SDK used in this project
SDK_LIBDIR	= lib
SDK_LDDIR	= ld
SDK_INCDIR	= include include/json

# we create two different files for uploading into the flash
# these are the names and options to generate them
USER1_BIN	= user1
USER2_BIN	= user2

# select which tools to use as compiler, librarian and linker
CC		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
CXX		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-g++
AR		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar
LD		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-g++
OBJCOPY := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objcopy



####
#### no user configurable options below here
####
FW_TOOL		?= python $(SDK_BASE)/tools/gen_appbin.py
SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))

SDK_LIBDIR	:= $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR))
SDK_INCDIR	:= $(addprefix -I$(SDK_BASE)/,$(SDK_INCDIR))

SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c*))
C_OBJ		:= $(patsubst %.c,%.o,$(SRC))
CXX_OBJ		:= $(patsubst %.cpp,%.o,$(C_OBJ))
OBJ		:= $(patsubst %.o,$(BUILD_BASE)/%.o,$(CXX_OBJ))
LIBS		:= $(addprefix -l,$(LIBS))
APP_AR		:= $(addprefix $(BUILD_BASE)/,$(TARGET)_app.a)
TARGET_OUT	:= $(addprefix $(BUILD_BASE)/,$(TARGET).out)

LD_SCRIPT	:= $(addprefix -T$(SDK_BASE)/$(SDK_LDDIR)/,$(LD_SCRIPT))

INCDIR	:= $(addprefix -I,$(SRC_DIR))
EXTRA_INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))
MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))

USER1_BIN	:= $(addprefix $(FW_BASE)/,$(USER1_BIN).bin)
USER2_BIN	:= $(addprefix $(FW_BASE)/,$(USER2_BIN).bin)

V ?= $(VERBOSE)
ifeq ("$(V)","1")
Q :=
vecho := @true
else
Q := @
vecho := @echo
endif

vpath %.c $(SRC_DIR)
vpath %.cpp $(SRC_DIR)

define compile-objects
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $$< -o $$@
$1/%.o: %.cpp
	$(vecho) "C+ $$<"
	$(Q) $(CXX) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CXXFLAGS)  -c $$< -o $$@
endef

.PHONY: all checkdirs clean

all: checkdirs $(TARGET_OUT) $(USER1_BIN) $(USER2_BIN)

base-bin: $(TARGET_OUT)
	$(vecho) "FW $@"	
	$(Q) $(OBJCOPY) --only-section .text -O binary $< $(BUILD_BASE)/eagle.app.v6.text.bin
	$(Q) $(OBJCOPY) --only-section .data -O binary $< $(BUILD_BASE)/eagle.app.v6.data.bin
	$(Q) $(OBJCOPY) --only-section .rodata -O binary $< $(BUILD_BASE)/eagle.app.v6.rodata.bin
	$(Q) $(OBJCOPY) --only-section .irom0.text -O binary $< $(BUILD_BASE)/eagle.app.v6.irom0text.bin

$(USER1_BIN): base-bin
	$(Q) cd $(BUILD_BASE); $(FW_TOOL) $(TARGET_OUT) 2 0 0 6 1; mv eagle.app.flash.bin $@
$(USER2_BIN): base-bin
	$(Q) cd $(BUILD_BASE); $(FW_TOOL) $(TARGET_OUT) 2 0 0 6 2; mv eagle.app.flash.bin $@


$(TARGET_OUT): $(APP_AR)
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@

$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^

checkdirs: $(BUILD_DIR) $(FW_BASE)

$(BUILD_DIR):
	$(Q) mkdir -p $@

flash: $(USER1_BIN) $(USER2_BIN)
	$(ESPTOOL) --port COM3 write_flash -ff 40m -fm qio -fs 32m-c1 0x00 $(SDK_BASE)/bin/boot_v1.6.bin 0x1000 $< 0x3fe000 $(SDK_BASE)/bin/blank.bin 0xfe000 $(SDK_BASE)/bin/blank.bin 0x3fc000 $(SDK_BASE)/bin/esp_init_data_default.bin 0xfc000 $(SDK_BASE)/bin/esp_init_data_default.bin

rest: $(USER1_BIN) $(USER2_BIN)
	$(Q) rm /f/projekti/doma/termostat/source/rest/routes/esp_firmwares/user*.bin
	$(Q) mv $(USER1_BIN) /f/projekti/doma/termostat/source/rest/routes/esp_firmwares/user1.bin
	$(Q) mv $(USER2_BIN) /f/projekti/doma/termostat/source/rest/routes/esp_firmwares/user2.bin

clean:
	$(Q) rm -rf $(BUILD_BASE)
	# $(Q) rm -f $(APP_AR)
	# $(Q) rm -f $(TARGET_OUT)
	# $(Q) rm -rf $(BUILD_DIR)
	# $(Q) rm -rf $(BUILD_BASE)
	# $(Q) rm -f $(USER1_BIN)
	# $(Q) rm -rf $(FW_BASE)

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))