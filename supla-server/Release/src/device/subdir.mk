################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/device/abstract_action_executor.cpp \
../src/device/abstract_gate_state_getter.cpp \
../src/device/action_executor.cpp \
../src/device/action_gate_openclose.cpp \
../src/device/action_gate_openclose_search_condition.cpp \
../src/device/device.cpp \
../src/device/devicechannel.cpp 

OBJS += \
./src/device/abstract_action_executor.o \
./src/device/abstract_gate_state_getter.o \
./src/device/action_executor.o \
./src/device/action_gate_openclose.o \
./src/device/action_gate_openclose_search_condition.o \
./src/device/device.o \
./src/device/devicechannel.o 

CPP_DEPS += \
./src/device/abstract_action_executor.d \
./src/device/abstract_gate_state_getter.d \
./src/device/action_executor.d \
./src/device/action_gate_openclose.d \
./src/device/action_gate_openclose_search_condition.d \
./src/device/device.d \
./src/device/devicechannel.d 


# Each subdirectory must supply rules for building sources it contributes
src/device/%.o: ../src/device/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DSERVER_VERSION_23 -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


