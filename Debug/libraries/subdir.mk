################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libraries/fifo.c \
../libraries/memory_organization.c \
../libraries/scheduler.c 

OBJS += \
./libraries/fifo.o \
./libraries/memory_organization.o \
./libraries/scheduler.o 

C_DEPS += \
./libraries/fifo.d \
./libraries/memory_organization.d \
./libraries/scheduler.d 


# Each subdirectory must supply rules for building sources it contributes
libraries/%.o: ../libraries/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DNRF51 -DDEBUG_NRF_USER -DBLE_STACK_SUPPORT_REQD -DBOARD_PCA10001 -DSPI_MASTER_0_ENABLE -I"D:/Eclipse_workspace/GWatch" -I"D:/Eclipse_workspace/GWatch/nordic/miscellaneous/CMSIS" -I"D:/Eclipse_workspace/GWatch/nordic/miscellaneous" -I"D:/Eclipse_workspace/GWatch/hardware" -I"D:/Eclipse_workspace/GWatch/nordic/libraries" -I"D:/Eclipse_workspace/GWatch/nordic/ble" -I"D:/Eclipse_workspace/GWatch/nordic/drivers" -I"D:/Eclipse_workspace/GWatch/nordic/softdevice" -I"D:/Eclipse_workspace/GWatch/ble" -I"D:/Eclipse_workspace/GWatch/libraries" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


