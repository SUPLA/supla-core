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
../src/device/devicechannel.cpp \
../src/device/gate_state_getter.cpp 

OBJS += \
./src/device/abstract_action_executor.o \
./src/device/abstract_gate_state_getter.o \
./src/device/action_executor.o \
./src/device/action_gate_openclose.o \
./src/device/action_gate_openclose_search_condition.o \
./src/device/device.o \
./src/device/devicechannel.o \
./src/device/gate_state_getter.o 

CPP_DEPS += \
./src/device/abstract_action_executor.d \
./src/device/abstract_gate_state_getter.d \
./src/device/action_executor.d \
./src/device/action_gate_openclose.d \
./src/device/action_gate_openclose_search_condition.d \
./src/device/device.d \
./src/device/devicechannel.d \
./src/device/gate_state_getter.d 


# Each subdirectory must supply rules for building sources it contributes
src/device/%.o: ../src/device/%.cpp src/device/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


