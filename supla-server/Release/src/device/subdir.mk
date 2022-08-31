################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/device/abstract_device_dao.cpp \
../src/device/abstract_value_getter.cpp \
../src/device/channel_address.cpp \
../src/device/channel_electricity_measurement.cpp \
../src/device/channel_gate_value.cpp \
../src/device/channel_ic_measurement.cpp \
../src/device/channel_onoff_value.cpp \
../src/device/channel_rgbw_value.cpp \
../src/device/channel_rs_value.cpp \
../src/device/channel_temphum.cpp \
../src/device/channel_thermostat_measurement.cpp \
../src/device/channel_value.cpp \
../src/device/device.cpp \
../src/device/device_dao.cpp \
../src/device/devicechannel.cpp \
../src/device/value_getter.cpp 

CPP_DEPS += \
./src/device/abstract_device_dao.d \
./src/device/abstract_value_getter.d \
./src/device/channel_address.d \
./src/device/channel_electricity_measurement.d \
./src/device/channel_gate_value.d \
./src/device/channel_ic_measurement.d \
./src/device/channel_onoff_value.d \
./src/device/channel_rgbw_value.d \
./src/device/channel_rs_value.d \
./src/device/channel_temphum.d \
./src/device/channel_thermostat_measurement.d \
./src/device/channel_value.d \
./src/device/device.d \
./src/device/device_dao.d \
./src/device/devicechannel.d \
./src/device/value_getter.d 

OBJS += \
./src/device/abstract_device_dao.o \
./src/device/abstract_value_getter.o \
./src/device/channel_address.o \
./src/device/channel_electricity_measurement.o \
./src/device/channel_gate_value.o \
./src/device/channel_ic_measurement.o \
./src/device/channel_onoff_value.o \
./src/device/channel_rgbw_value.o \
./src/device/channel_rs_value.o \
./src/device/channel_temphum.o \
./src/device/channel_thermostat_measurement.o \
./src/device/channel_value.o \
./src/device/device.o \
./src/device/device_dao.o \
./src/device/devicechannel.o \
./src/device/value_getter.o 


# Each subdirectory must supply rules for building sources it contributes
src/device/%.o: ../src/device/%.cpp src/device/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-device

clean-src-2f-device:
	-$(RM) ./src/device/abstract_device_dao.d ./src/device/abstract_device_dao.o ./src/device/abstract_value_getter.d ./src/device/abstract_value_getter.o ./src/device/channel_address.d ./src/device/channel_address.o ./src/device/channel_electricity_measurement.d ./src/device/channel_electricity_measurement.o ./src/device/channel_gate_value.d ./src/device/channel_gate_value.o ./src/device/channel_ic_measurement.d ./src/device/channel_ic_measurement.o ./src/device/channel_onoff_value.d ./src/device/channel_onoff_value.o ./src/device/channel_rgbw_value.d ./src/device/channel_rgbw_value.o ./src/device/channel_rs_value.d ./src/device/channel_rs_value.o ./src/device/channel_temphum.d ./src/device/channel_temphum.o ./src/device/channel_thermostat_measurement.d ./src/device/channel_thermostat_measurement.o ./src/device/channel_value.d ./src/device/channel_value.o ./src/device/device.d ./src/device/device.o ./src/device/device_dao.d ./src/device/device_dao.o ./src/device/devicechannel.d ./src/device/devicechannel.o ./src/device/value_getter.d ./src/device/value_getter.o

.PHONY: clean-src-2f-device

