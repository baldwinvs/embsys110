################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/UserLed.bak/LedPattern.cpp \
../src/UserLed.bak/UserLed.cpp \
../src/UserLed.bak/UserLedCmd.cpp 

OBJS += \
./src/UserLed.bak/LedPattern.o \
./src/UserLed.bak/UserLed.o \
./src/UserLed.bak/UserLedCmd.o 

CPP_DEPS += \
./src/UserLed.bak/LedPattern.d \
./src/UserLed.bak/UserLed.d \
./src/UserLed.bak/UserLedCmd.d 


# Each subdirectory must supply rules for building sources it contributes
src/UserLed.bak/%.o: ../src/UserLed.bak/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -O3 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DTRACE -DSTM32F401xE -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f4xx" -I"../system/BSP/STM32F4xx-Nucleo" -I"../system/BSP/Components" -I../qpcpp/ports/arm-cm/qxk/gnu -I../qpcpp/include -I../qpcpp/src -I"../framework/include" -I"../src/Console" -I"../src/Console/CmdInput" -I"../src/Console/CmdParser" -I"../src/System" -I"../src/GpioInAct" -I"../src/GpioInAct/GpioIn" -I"../src/Wifi" -I"../src/Wifi/WifiSt" -I"../src/GpioOutAct" -I"../src/GpioOutAct/GpioOut" -I"../src/Disp" -I"../src/Disp/Adafruit" -I"../src/Disp/Adafruit/Fonts" -I"../src/Disp/Ili9341" -I"../src/Disp/LedPanel" -I"../src/Sensor" -I"../src/Sensor/Iks01a1" -I"../src/Sensor/Iks01a1/Iks01a1AccelGyro" -I"../src/Sensor/Iks01a1/Iks01a1HumidTemp" -I"../src/Sensor/Iks01a1/Iks01a1Mag" -I"../src/Sensor/Iks01a1/Iks01a1Press" -I"../src/Sensor/Iks01a1/BSP/X_NUCLEO_IKS01A1" -I"../src/Sensor/Iks01a1/BSP/Components/Common" -I"../src/Sensor/Iks01a1/BSP/Components/hts221" -I"../src/Sensor/Iks01a1/BSP/Components/lis3mdl" -I"../src/Sensor/Iks01a1/BSP/Components/lps22hb" -I"../src/Sensor/Iks01a1/BSP/Components/lps25hb" -I"../src/Sensor/Iks01a1/BSP/Components/lsm303agr" -I"../src/Sensor/Iks01a1/BSP/Components/lsm6ds0" -I"../src/Sensor/Iks01a1/BSP/Components/lsm6ds3" -I"../src/Sensor/Iks01a1/BSP/Components/lsm6dsl" -I"../src/Demo" -I"../src/AOWashingMachine" -I"../src/Traffic" -I"../src/Traffic/Lamp" -I"../src/LevelMeter" -I"../src/Template/CompositeAct" -I"../src/Template/CompositeAct/CompositeReg" -I"../src/Template/SimpleAct" -I"../src/Template/SimpleReg" -I"../src/UartAct" -I"../src/UartAct/UartIn" -I"../src/UartAct/UartOut" -std=gnu++11 -fabi-version=0 -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-threadsafe-statics -mfloat-abi=hard -mfpu=fpv4-sp-d16 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

