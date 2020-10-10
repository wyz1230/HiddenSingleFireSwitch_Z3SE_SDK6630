################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../switch_app/leds_app/leds-app.c 

OBJS += \
./switch_app/leds_app/leds-app.o 

C_DEPS += \
./switch_app/leds_app/leds-app.d 


# Each subdirectory must supply rules for building sources it contributes
switch_app/leds_app/leds-app.o: ../switch_app/leds_app/leds-app.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m33 -mthumb -std=c99 -O2 -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"switch_app/leds_app/leds-app.d" -MT"switch_app/leds_app/leds-app.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


