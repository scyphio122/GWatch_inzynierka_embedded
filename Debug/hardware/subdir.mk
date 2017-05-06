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
../hardware/ext_flash.c \
../hardware/int_flash.c \
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
./hardware/ext_flash.o \
./hardware/int_flash.o \
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
./hardware/ext_flash.d \
./hardware/int_flash.d \
./hardware/spi.d \
./hardware/timer.d 


# Each subdirectory must supply rules for building sources it contributes
hardware/%.o: ../hardware/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DNRF51 -DDEBUG_NRF_USER -DBLE_STACK_SUPPORT_REQD -DBOARD_PCA10001 -DSPI_MASTER_0_ENABLE -I"D:/Eclipse_workspace/GWatch/nordic/miscellaneous/CMSIS" -I"D:/Eclipse_workspace/GWatch/nordic/miscellaneous" -I"D:/Eclipse_workspace/GWatch" -I"D:/Eclipse_workspace/GWatch/hardware" -I"D:/Eclipse_workspace/GWatch/nordic/libraries" -I"D:/Eclipse_workspace/GWatch/nordic/ble" -I"D:/Eclipse_workspace/GWatch/nordic/drivers" -I"D:/Eclipse_workspace/GWatch/nordic/softdevice" -I"D:/Eclipse_workspace/GWatch/ble" -I"D:/Eclipse_workspace/GWatch/libraries" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


