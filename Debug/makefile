################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include libraries/subdir.mk
-include hardware/subdir.mk
-include fifo/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
GWatch.hex \

SECONDARY_SIZE += \
GWatch.siz \


# All Target
all: GWatch.elf secondary-outputs

# Tool invocations
GWatch.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross ARM C Linker'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -T "D:\Eclipse_workspace\packs_short/Softdevice_S110/armgcc_s110_nrf51422_xxaa.ld" -Xlinker --gc-sections -L"D:\Eclipse_workspace\packs_short/Softdevice_S110" -L"D:\Eclipse_workspace\packs_short" -L"D:\Eclipse_workspace\packs_short/nRF_Libraries" -L"D:\Eclipse_workspace\packs_short/nRF_Drivers" -L"D:\Eclipse_workspace\packs_short/nRF_BLE" -Wl,-Map,"GWatch.map" -o "GWatch.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

GWatch.hex: GWatch.elf
	@echo 'Invoking: Cross ARM GNU Create Flash Image'
	arm-none-eabi-objcopy -O ihex "GWatch.elf"  "GWatch.hex"
	@echo 'Finished building: $@'
	@echo ' '

GWatch.siz: GWatch.elf
	@echo 'Invoking: Cross ARM GNU Print Size'
	arm-none-eabi-size --format=berkeley "GWatch.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(SECONDARY_FLASH)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_UPPER_DEPS)$(C_DEPS) GWatch.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
