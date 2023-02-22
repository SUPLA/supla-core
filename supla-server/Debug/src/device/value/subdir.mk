################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/device/value/channel_binary_sensor_value.cpp \
../src/device/value/channel_floating_point_sensor_value.cpp \
../src/device/value/channel_gate_value.cpp \
../src/device/value/channel_onoff_value.cpp \
../src/device/value/channel_rgbw_value.cpp \
../src/device/value/channel_rs_value.cpp \
../src/device/value/channel_temphum_value.cpp \
../src/device/value/channel_value.cpp \
../src/device/value/channel_value_envelope.cpp \
../src/device/value/channel_valve_value.cpp 

CPP_DEPS += \
./src/device/value/channel_binary_sensor_value.d \
./src/device/value/channel_floating_point_sensor_value.d \
./src/device/value/channel_gate_value.d \
./src/device/value/channel_onoff_value.d \
./src/device/value/channel_rgbw_value.d \
./src/device/value/channel_rs_value.d \
./src/device/value/channel_temphum_value.d \
./src/device/value/channel_value.d \
./src/device/value/channel_value_envelope.d \
./src/device/value/channel_valve_value.d 

OBJS += \
./src/device/value/channel_binary_sensor_value.o \
./src/device/value/channel_floating_point_sensor_value.o \
./src/device/value/channel_gate_value.o \
./src/device/value/channel_onoff_value.o \
./src/device/value/channel_rgbw_value.o \
./src/device/value/channel_rs_value.o \
./src/device/value/channel_temphum_value.o \
./src/device/value/channel_value.o \
./src/device/value/channel_value_envelope.o \
./src/device/value/channel_valve_value.o 


# Each subdirectory must supply rules for building sources it contributes
src/device/value/%.o: ../src/device/value/%.cpp src/device/value/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-device-2f-value

clean-src-2f-device-2f-value:
	-$(RM) ./src/device/value/channel_binary_sensor_value.d ./src/device/value/channel_binary_sensor_value.o ./src/device/value/channel_floating_point_sensor_value.d ./src/device/value/channel_floating_point_sensor_value.o ./src/device/value/channel_gate_value.d ./src/device/value/channel_gate_value.o ./src/device/value/channel_onoff_value.d ./src/device/value/channel_onoff_value.o ./src/device/value/channel_rgbw_value.d ./src/device/value/channel_rgbw_value.o ./src/device/value/channel_rs_value.d ./src/device/value/channel_rs_value.o ./src/device/value/channel_temphum_value.d ./src/device/value/channel_temphum_value.o ./src/device/value/channel_value.d ./src/device/value/channel_value.o ./src/device/value/channel_value_envelope.d ./src/device/value/channel_value_envelope.o ./src/device/value/channel_valve_value.d ./src/device/value/channel_valve_value.o

.PHONY: clean-src-2f-device-2f-value

