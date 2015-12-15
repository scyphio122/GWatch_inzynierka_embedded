################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../hardware/Clock.c \
../hardware/GPS.c \
../hardware/RTC.c \
../hardware/UART.c \
../hardware/WATCHDOG.c \
../hardware/adc.c \
../hardware/display.c \
../hardware/spi.c \
../hardware/timer.c 

OBJS += \
./hardware/Clock.o \
./hardware/GPS.o \
./hardware/RTC.o \
./hardware/UART.o \
./hardware/WATCHDOG.o \
./hardware/adc.o \
./hardware/display.o \
./hardware/spi.o \
./hardware/timer.o 

C_DEPS += \
./hardware/Clock.d \
./hardware/GPS.d \
./hardware/RTC.d \
./hardware/UART.d \
./hardware/WATCHDOG.d \
./hardware/adc.d \
./hardware/display.d \
./hardware/spi.d \
./hardware/timer.d 


# Each subdirectory must supply rules for building sources it contributes
hardware/%.o: ../hardware/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DNRF51 -DDEBUG_NRF_USER -DBLE_STACK_SUPPORT_REQD -DBOARD_PCA10001 -DSPI_MASTER_0_ENABLE -I"D:/Eclipse_workspace/GWatch" -I"D:\Eclipse_workspace\packs_short/CMSIS" -I"D:/Eclipse_workspace/GWatch/libs" -I"D:/Eclipse_workspace/GWatch/hardware" -I"D:/Eclipse_workspace/GWatch/nordic/libraries" -I"D:/Eclipse_workspace/GWatch/nordic/ble" -I"D:/Eclipse_workspace/GWatch/nordic/drivers" -I"D:/Eclipse_workspace/GWatch/nordic/softdevice" -I"D:\Eclipse_workspace\packs_short/nRF_Drivers" -I"D:\Eclipse_workspace\packs_short/SoftDevice_common" -I"D:\Eclipse_workspace\packs_short/nRF_BLE" -I"D:\Eclipse_workspace\packs_short/Softdevice_S110" -I"D:\Eclipse_workspace\packs_short/nRF_Boards" -I"D:\Eclipse_workspace\packs_short/nRF_Libraries/3.0.1/util" -I"D:\Eclipse_workspace\packs_short/nRF_Libraries/3.0.1/fifo" -I"D:\Eclipse_workspace\packs_short/nRF_Libraries/3.0.1/trace" -I"D:\Eclipse_workspace\packs_short/nRF_Libraries/3.0.1/timer" -I"D:\Eclipse_workspace\packs_short/nRF_Libraries/3.0.1/scheduler" -I"D:\Eclipse_workspace\packs_short/nRF_Libraries/3.0.1/crc16" -I"D:/Eclipse_workspace/GWatch/ble" -I"D:\Eclipse_workspace\packs_short/DEVICE" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


