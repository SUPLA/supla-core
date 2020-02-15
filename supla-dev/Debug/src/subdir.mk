################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/channel-io.c \
../src/common_dht_read.c \
../src/devcfg.c \
../src/devconnection.c \
../src/gpio.c \
../src/mcp23008.c \
../src/pi_2_dht_read.c \
../src/pi_2_mmio.c \
../src/w1.c 

CPP_SRCS += \
../src/accept_loop.cpp \
../src/client_publisher.cpp \
../src/client_subscriber.cpp \
../src/ipcctrl.cpp \
../src/supla-dev.cpp 

OBJS += \
./src/accept_loop.o \
./src/channel-io.o \
./src/client_publisher.o \
./src/client_subscriber.o \
./src/common_dht_read.o \
./src/devcfg.o \
./src/devconnection.o \
./src/gpio.o \
./src/ipcctrl.o \
./src/mcp23008.o \
./src/pi_2_dht_read.o \
./src/pi_2_mmio.o \
./src/supla-dev.o \
./src/w1.o 

C_DEPS += \
./src/channel-io.d \
./src/common_dht_read.d \
./src/devcfg.d \
./src/devconnection.d \
./src/gpio.d \
./src/mcp23008.d \
./src/pi_2_dht_read.d \
./src/pi_2_mmio.d \
./src/w1.d 

CPP_DEPS += \
./src/accept_loop.d \
./src/client_publisher.d \
./src/client_subscriber.d \
./src/ipcctrl.d \
./src/supla-dev.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__NO_DATABASE -D__NO_USER -I/usr/src/openssl/include -I../src/json -O0 -g3 -Wall -fsigned-char -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -D__DEBUG=1 -I/usr/src/openssl/include -I../src/json -O0 -g3 -Wall -fsigned-char -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


