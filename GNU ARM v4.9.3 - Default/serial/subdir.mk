################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v2.6/platform/base/hal/plugin/serial/cortexm/efm32/com.c 

OBJS += \
./serial/com.o 

C_DEPS += \
./serial/com.d 


# Each subdirectory must supply rules for building sources it contributes
serial/com.o: C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v2.6/platform/base/hal/plugin/serial/cortexm/efm32/com.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m33 -mthumb -std=c99 -O2 -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"serial/com.d" -MT"serial/com.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


