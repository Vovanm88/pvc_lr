################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Src/drivers/Buzzer.cpp \
../Core/Src/drivers/Display.cpp \
../Core/Src/drivers/Keyboard.cpp \
../Core/Src/drivers/Synthesizer.cpp \
../Core/Src/drivers/Uart.cpp 

OBJS += \
./Core/Src/drivers/Buzzer.o \
./Core/Src/drivers/Display.o \
./Core/Src/drivers/Keyboard.o \
./Core/Src/drivers/Synthesizer.o \
./Core/Src/drivers/Uart.o 

CPP_DEPS += \
./Core/Src/drivers/Buzzer.d \
./Core/Src/drivers/Display.d \
./Core/Src/drivers/Keyboard.d \
./Core/Src/drivers/Synthesizer.d \
./Core/Src/drivers/Uart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/drivers/%.o Core/Src/drivers/%.su: ../Core/Src/drivers/%.cpp Core/Src/drivers/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F427xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-drivers

clean-Core-2f-Src-2f-drivers:
	-$(RM) ./Core/Src/drivers/Buzzer.d ./Core/Src/drivers/Buzzer.o ./Core/Src/drivers/Buzzer.su ./Core/Src/drivers/Display.d ./Core/Src/drivers/Display.o ./Core/Src/drivers/Display.su ./Core/Src/drivers/Keyboard.d ./Core/Src/drivers/Keyboard.o ./Core/Src/drivers/Keyboard.su ./Core/Src/drivers/Synthesizer.d ./Core/Src/drivers/Synthesizer.o ./Core/Src/drivers/Synthesizer.su ./Core/Src/drivers/Uart.d ./Core/Src/drivers/Uart.o ./Core/Src/drivers/Uart.su

.PHONY: clean-Core-2f-Src-2f-drivers

