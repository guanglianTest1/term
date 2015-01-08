################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../HttpModule.c \
../Timer.c \
../appSqlite.c \
../cJSON.c \
../json.c \
../main.c \
../net.c \
../sysinit.c \
../term.c 

OBJS += \
./HttpModule.o \
./Timer.o \
./appSqlite.o \
./cJSON.o \
./json.o \
./main.o \
./net.o \
./sysinit.o \
./term.o 

C_DEPS += \
./HttpModule.d \
./Timer.d \
./appSqlite.d \
./cJSON.d \
./json.d \
./main.d \
./net.d \
./sysinit.d \
./term.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-arago-linux-gnueabi-gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


