################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/clientcfg.c 

CPP_SRCS += \
../src/client_command.cpp \
../src/client_config.cpp \
../src/client_device.cpp \
../src/client_loop.cpp \
../src/client_publisher.cpp \
../src/client_state.cpp \
../src/client_subscriber.cpp \
../src/common.cpp 

OBJS += \
./src/client_command.o \
./src/client_config.o \
./src/client_device.o \
./src/client_loop.o \
./src/client_publisher.o \
./src/client_state.o \
./src/client_subscriber.o \
./src/clientcfg.o \
./src/common.o 

C_DEPS += \
./src/clientcfg.d 

CPP_DEPS += \
./src/client_command.d \
./src/client_config.d \
./src/client_device.d \
./src/client_loop.d \
./src/client_publisher.d \
./src/client_state.d \
./src/client_subscriber.d \
./src/common.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++11 -D__DEBUG=1 -D__NO_USER -D__NO_DATABASE -I$(SSLDIR)/include -I../src/json -I../src/test -O0 -g3 -Wall -fsigned-char  -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -D__DEBUG=1 -D__NO_DATABASE -D__NO_USER -I$(SSLDIR)/include -I../src/json -O0 -g3 -Wall -fsigned-char  -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


