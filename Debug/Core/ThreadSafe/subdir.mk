################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/ThreadSafe/newlib_lock_glue.c 

OBJS += \
./Core/ThreadSafe/newlib_lock_glue.o 

C_DEPS += \
./Core/ThreadSafe/newlib_lock_glue.d 


# Each subdirectory must supply rules for building sources it contributes
Core/ThreadSafe/%.o Core/ThreadSafe/%.su: ../Core/ThreadSafe/%.c Core/ThreadSafe/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F469xx -DSTM32_THREAD_SAFE_STRATEGY=5 -c -I../Core/Inc -I../FATFS/Target -I../FATFS/App -I../USB_HOST/App -I../USB_HOST/Target -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Core/ThreadSafe -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-ThreadSafe

clean-Core-2f-ThreadSafe:
	-$(RM) ./Core/ThreadSafe/newlib_lock_glue.d ./Core/ThreadSafe/newlib_lock_glue.o ./Core/ThreadSafe/newlib_lock_glue.su

.PHONY: clean-Core-2f-ThreadSafe

