################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Src/synthesizer/AudioToBuzzerAdapter.cpp \
../Core/Src/synthesizer/SynthesizerBridge.cpp \
../Core/Src/synthesizer/VoiceMixer.cpp \
../Core/Src/synthesizer/WaveGenerator.cpp \
../Core/Src/synthesizer/WaveSynthesizer.cpp 

OBJS += \
./Core/Src/synthesizer/AudioToBuzzerAdapter.o \
./Core/Src/synthesizer/SynthesizerBridge.o \
./Core/Src/synthesizer/VoiceMixer.o \
./Core/Src/synthesizer/WaveGenerator.o \
./Core/Src/synthesizer/WaveSynthesizer.o 

CPP_DEPS += \
./Core/Src/synthesizer/AudioToBuzzerAdapter.d \
./Core/Src/synthesizer/SynthesizerBridge.d \
./Core/Src/synthesizer/VoiceMixer.d \
./Core/Src/synthesizer/WaveGenerator.d \
./Core/Src/synthesizer/WaveSynthesizer.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/synthesizer/%.o Core/Src/synthesizer/%.su: ../Core/Src/synthesizer/%.cpp Core/Src/synthesizer/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F427xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-synthesizer

clean-Core-2f-Src-2f-synthesizer:
	-$(RM) ./Core/Src/synthesizer/AudioToBuzzerAdapter.d ./Core/Src/synthesizer/AudioToBuzzerAdapter.o ./Core/Src/synthesizer/AudioToBuzzerAdapter.su ./Core/Src/synthesizer/SynthesizerBridge.d ./Core/Src/synthesizer/SynthesizerBridge.o ./Core/Src/synthesizer/SynthesizerBridge.su ./Core/Src/synthesizer/VoiceMixer.d ./Core/Src/synthesizer/VoiceMixer.o ./Core/Src/synthesizer/VoiceMixer.su ./Core/Src/synthesizer/WaveGenerator.d ./Core/Src/synthesizer/WaveGenerator.o ./Core/Src/synthesizer/WaveGenerator.su ./Core/Src/synthesizer/WaveSynthesizer.d ./Core/Src/synthesizer/WaveSynthesizer.o ./Core/Src/synthesizer/WaveSynthesizer.su

.PHONY: clean-Core-2f-Src-2f-synthesizer

