##########################################################################################################################
# File automatically-generated by tool: [projectgenerator] version: [3.17.1] date: [Wed Nov 23 20:51:47 CST 2022]
##########################################################################################################################

# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#	2017-02-10 - Several enhancements + project update mode
#   2015-07-22 - first version
# ------------------------------------------------

######################################
# target
######################################
TARGET = STM32_STD


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og
# C standard
C_STANDARD = c17

#######################################
# paths
#######################################
# Build path
BUILD_DIR = OBJ

######################################
# source
######################################
# FatFs sources
FATFS_SOURCES = \
DRIVER/FatFs/source/ff.c \
DRIVER/FatFs/source/diskio.c \
DRIVER/FatFs/source/ffunicode.c

# C sources
C_SOURCES =  \
CORE/core_cm3.c \
STM32F10x_FWLib/src/misc.c \
STM32F10x_FWLib/src/stm32f10x_rcc.c \
STM32F10x_FWLib/src/stm32f10x_gpio.c \
STM32F10x_FWLib/src/stm32f10x_usart.c \
STM32F10x_FWLib/src/stm32f10x_exti.c \
STM32F10x_FWLib/src/stm32f10x_iwdg.c \
STM32F10x_FWLib/src/stm32f10x_wwdg.c \
STM32F10x_FWLib/src/stm32f10x_tim.c \
STM32F10x_FWLib/src/stm32f10x_dma.c \
STM32F10x_FWLib/src/stm32f10x_i2c.c \
STM32F10x_FWLib/src/stm32f10x_spi.c \
STM32F10x_FWLib/src/stm32f10x_fsmc.c \
STM32F10x_FWLib/src/stm32f10x_sdio.c \
SYSTEM/delay/delay.c \
SYSTEM/sys/sys.c \
${FATFS_SOURCES} \
USER/stm32f10x_it.c \
USER/system_stm32f10x.c \
USER/main.c \
HW/LED/LED.c \
HW/BEEP/BEEP.c \
HW/KEY/KEY.c \
HW/USART/USART.c \
HW/EXTI/exti.c \
HW/IWDG/iwdg.c \
HW/WWDG/wwdg.c \
HW/TIM/IT/tim.c \
HW/TIM/PWM_OUT/pwm_out.c \
HW/TIM/CAP/cap.c \
HW/TPAD/tpad.c \
HW/DMA/dma.c \
HW/EEPROM/I2C_Software/i2c_software.c \
HW/EEPROM/I2C_Hardware/i2c_hardware.c \
HW/FLASH/External_Flash/exFlash.c \
HW/FLASH/External_Flash_FatFs/exFlashFatFs.c \
HW/LCD/lcd.c \
HW/SDCard/stm32_eval_sdio_sd.c \
HW/SDCard/SDCard.c \
HW/LCD/Fonts/Song_Unicode_Resolution_16_16/Flash_Download.c \
HW/LCD/Fonts/Song_Unicode_Resolution_16_16/Flash_Font.c

# ASM sources
ASM_SOURCES =  \
CORE/startup_stm32f103xe.s


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m3

# fpu
# NONE for Cortex-M0/M0+/M3

# float-abi


# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_STDPERIPH_DRIVER \
-DSTM32F10X_HD

# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-ICore \
-ISTM32F10x_FWLib/inc \
-ISYSTEM/delay \
-IDRIVER/FatFs/source \
-IDRIVER/STM32_USB-FS-Device_Lib_V4.1.0/STM32_USB-FS-Device_Driver/inc \
-IDRIVER/STM32_USB-FS-Device_Lib_V4.1.0/Virtual_COM_Port/inc \
-IUSER \
-ISYSTEM/sys \
-IHW/LED \
-IHW/KEY \
-IHW/FLASH/External_Flash \
-IHW/LCD \
-IHW/SDCard \
-IHW/LCD/Fonts/Song_Unicode_Resolution_16_16

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -std=$(C_STANDARD)

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = CORE/STM32F103ZETx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***