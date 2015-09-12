################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/cfg.c \
../src/channel-io.c \
../src/devcfg.c \
../src/devconnection.c \
../src/eh.c \
../src/gpio.c \
../src/ini.c \
../src/lck.c \
../src/log.c \
../src/proto.c \
../src/safearray.c \
../src/srpc.c \
../src/sthread.c \
../src/supla-dev.c \
../src/supla-socket.c \
../src/tools.c \
../src/w1.c 

OBJS += \
./src/cfg.o \
./src/channel-io.o \
./src/devcfg.o \
./src/devconnection.o \
./src/eh.o \
./src/gpio.o \
./src/ini.o \
./src/lck.o \
./src/log.o \
./src/proto.o \
./src/safearray.o \
./src/srpc.o \
./src/sthread.o \
./src/supla-dev.o \
./src/supla-socket.o \
./src/tools.o \
./src/w1.o 

C_DEPS += \
./src/cfg.d \
./src/channel-io.d \
./src/devcfg.d \
./src/devconnection.d \
./src/eh.d \
./src/gpio.d \
./src/ini.d \
./src/lck.d \
./src/log.d \
./src/proto.d \
./src/safearray.d \
./src/srpc.d \
./src/sthread.d \
./src/supla-dev.d \
./src/supla-socket.d \
./src/tools.d \
./src/w1.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


