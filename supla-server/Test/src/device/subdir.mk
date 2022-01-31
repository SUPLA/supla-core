################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/device/abstract_gate_state_getter.cpp \
../src/device/abstract_value_getter.cpp \
../src/device/channel_gate_value.cpp \
../src/device/channel_rs_value.cpp \
../src/device/channel_value.cpp \
../src/device/device.cpp \
../src/device/devicechannel.cpp \
../src/device/gate_state_getter.cpp \
../src/device/value_getter.cpp 

CPP_DEPS += \
./src/device/abstract_gate_state_getter.d \
./src/device/abstract_value_getter.d \
./src/device/channel_gate_value.d \
./src/device/channel_rs_value.d \
./src/device/channel_value.d \
./src/device/device.d \
./src/device/devicechannel.d \
./src/device/gate_state_getter.d \
./src/device/value_getter.d 

OBJS += \
./src/device/abstract_gate_state_getter.o \
./src/device/abstract_value_getter.o \
./src/device/channel_gate_value.o \
./src/device/channel_rs_value.o \
./src/device/channel_value.o \
./src/device/device.o \
./src/device/devicechannel.o \
./src/device/gate_state_getter.o \
./src/device/value_getter.o 


# Each subdirectory must supply rules for building sources it contributes
src/device/%.o: ../src/device/%.cpp src/device/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-device

clean-src-2f-device:
	-$(RM) ./src/device/abstract_gate_state_getter.d ./src/device/abstract_gate_state_getter.o ./src/device/abstract_value_getter.d ./src/device/abstract_value_getter.o ./src/device/channel_gate_value.d ./src/device/channel_gate_value.o ./src/device/channel_rs_value.d ./src/device/channel_rs_value.o ./src/device/channel_value.d ./src/device/channel_value.o ./src/device/device.d ./src/device/device.o ./src/device/devicechannel.d ./src/device/devicechannel.o ./src/device/gate_state_getter.d ./src/device/gate_state_getter.o ./src/device/value_getter.d ./src/device/value_getter.o

.PHONY: clean-src-2f-device

