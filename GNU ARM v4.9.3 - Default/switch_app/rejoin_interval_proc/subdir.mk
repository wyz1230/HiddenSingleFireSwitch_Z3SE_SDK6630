################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../switch_app/rejoin_interval_proc/rejoin-interval-proc\ -\ ֧��ÿ���ŵ���������.c \
../switch_app/rejoin_interval_proc/rejoin-interval-proc.c \
../switch_app/rejoin_interval_proc/rejoin-interval-proc_�ɰ汾.c 

OBJS += \
./switch_app/rejoin_interval_proc/rejoin-interval-proc\ -\ ֧��ÿ���ŵ���������.o \
./switch_app/rejoin_interval_proc/rejoin-interval-proc.o \
./switch_app/rejoin_interval_proc/rejoin-interval-proc_�ɰ汾.o 

C_DEPS += \
./switch_app/rejoin_interval_proc/rejoin-interval-proc\ -\ ֧��ÿ���ŵ���������.d \
./switch_app/rejoin_interval_proc/rejoin-interval-proc.d \
./switch_app/rejoin_interval_proc/rejoin-interval-proc_�ɰ汾.d 


# Each subdirectory must supply rules for building sources it contributes
switch_app/rejoin_interval_proc/rejoin-interval-proc\ -\ ֧��ÿ���ŵ���������.o: ../switch_app/rejoin_interval_proc/rejoin-interval-proc\ -\ ֧��ÿ���ŵ���������.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m33 -mthumb -std=c99 -O2 -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"switch_app/rejoin_interval_proc/rejoin-interval-proc - ֧��ÿ���ŵ���������.d" -MT"switch_app/rejoin_interval_proc/rejoin-interval-proc\ -\ ֧��ÿ���ŵ���������.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

switch_app/rejoin_interval_proc/rejoin-interval-proc.o: ../switch_app/rejoin_interval_proc/rejoin-interval-proc.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m33 -mthumb -std=c99 -O2 -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"switch_app/rejoin_interval_proc/rejoin-interval-proc.d" -MT"switch_app/rejoin_interval_proc/rejoin-interval-proc.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

switch_app/rejoin_interval_proc/rejoin-interval-proc_�ɰ汾.o: ../switch_app/rejoin_interval_proc/rejoin-interval-proc_�ɰ汾.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m33 -mthumb -std=c99 -O2 -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"switch_app/rejoin_interval_proc/rejoin-interval-proc_�ɰ汾.d" -MT"switch_app/rejoin_interval_proc/rejoin-interval-proc_�ɰ汾.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


