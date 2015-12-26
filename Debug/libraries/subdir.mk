################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libraries/fifo.c \
../libraries/memory_organization.c 

OBJS += \
./libraries/fifo.o \
./libraries/memory_organization.o 

C_DEPS += \
./libraries/fifo.d \
./libraries/memory_organization.d 


# Each subdirectory must supply rules for building sources it contributes
libraries/%.o: ../libraries/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DNRF51 -DDEBUG_NRF_USER -DBLE_STACK_SUPPORT_REQD -DBOARD_PCA10001 -DSPI_MASTER_0_ENABLE -I"D:/Eclipse_workspace/GWatch" -I"D:\Eclipse_workspace\packs_short/CMSIS" -I"D:/Eclipse_workspace/GWatch/libs" -I"D:/Eclipse_workspace/GWatch/hardware" -I"D:/Eclipse_workspace/GWatch/nordic/libraries" -I"D:/Eclipse_workspace/GWatch/nordic/ble" -I"D:/Eclipse_workspace/GWatch/nordic/drivers" -I"D:/Eclipse_workspace/GWatch/nordic/softdevice" -I"D:\Eclipse_workspace\packs_short/nRF_Drivers" -I"D:\Eclipse_workspace\packs_short/SoftDevice_common" -I"D:\Eclipse_workspace\packs_short/nRF_BLE" -I"D:\Eclipse_workspace\packs_short/Softdevice_S110" -I"D:\Eclipse_workspace\packs_short/nRF_Boards" -I"D:\Eclipse_workspace\packs_short/nRF_Libraries/3.0.1/util" -I"D:\Eclipse_workspace\packs_short/nRF_Libraries/3.0.1/fifo" -I"D:\Eclipse_workspace\packs_short/nRF_Libraries/3.0.1/trace" -I"D:\Eclipse_workspace\packs_short/nRF_Libraries/3.0.1/timer" -I"D:\Eclipse_workspace\packs_short/nRF_Libraries/3.0.1/scheduler" -I"D:\Eclipse_workspace\packs_short/nRF_Libraries/3.0.1/crc16" -I"D:/Eclipse_workspace/GWatch/ble" -I"D:\Eclipse_workspace\packs_short/DEVICE" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


