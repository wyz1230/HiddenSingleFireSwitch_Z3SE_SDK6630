################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../switch_app/app-callback.c \
../switch_app/scenes-recall-transition-proc.c 

OBJS += \
./switch_app/app-callback.o \
./switch_app/scenes-recall-transition-proc.o 

C_DEPS += \
./switch_app/app-callback.d \
./switch_app/scenes-recall-transition-proc.d 


# Each subdirectory must supply rules for building sources it contributes
switch_app/app-callback.o: ../switch_app/app-callback.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m33 -mthumb -std=c99 -O2 -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"switch_app/app-callback.d" -MT"switch_app/app-callback.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

switch_app/scenes-recall-transition-proc.o: ../switch_app/scenes-recall-transition-proc.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m33 -mthumb -std=c99 -O2 -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"switch_app/scenes-recall-transition-proc.d" -MT"switch_app/scenes-recall-transition-proc.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


