################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../HiddenSingleFireSwitch_Z3SE_SDK6630_callbacks.c \
../callback-stub.c \
../stack-handler-stub.c 

OBJS += \
./HiddenSingleFireSwitch_Z3SE_SDK6630_callbacks.o \
./callback-stub.o \
./stack-handler-stub.o 

C_DEPS += \
./HiddenSingleFireSwitch_Z3SE_SDK6630_callbacks.d \
./callback-stub.d \
./stack-handler-stub.d 


# Each subdirectory must supply rules for building sources it contributes
HiddenSingleFireSwitch_Z3SE_SDK6630_callbacks.o: ../HiddenSingleFireSwitch_Z3SE_SDK6630_callbacks.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m33 -mthumb -std=c99 -O2 -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"HiddenSingleFireSwitch_Z3SE_SDK6630_callbacks.d" -MT"HiddenSingleFireSwitch_Z3SE_SDK6630_callbacks.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

callback-stub.o: ../callback-stub.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m33 -mthumb -std=c99 -O2 -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"callback-stub.d" -MT"callback-stub.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

stack-handler-stub.o: ../stack-handler-stub.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m33 -mthumb -std=c99 -O2 -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"stack-handler-stub.d" -MT"stack-handler-stub.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


