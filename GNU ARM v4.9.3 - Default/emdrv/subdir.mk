################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v2.6/platform/emdrv/uartdrv/src/uartdrv.c 

OBJS += \
./emdrv/uartdrv.o 

C_DEPS += \
./emdrv/uartdrv.d 


# Each subdirectory must supply rules for building sources it contributes
emdrv/uartdrv.o: C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v2.6/platform/emdrv/uartdrv/src/uartdrv.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m33 -mthumb -std=c99 -O2 -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"emdrv/uartdrv.d" -MT"emdrv/uartdrv.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


