################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/device/value/abstract_channel_value.cpp \
../src/device/value/channel_binary_sensor_value.cpp \
../src/device/value/channel_container_value.cpp \
../src/device/value/channel_dgf_value.cpp \
../src/device/value/channel_em_value.cpp \
../src/device/value/channel_fb_value.cpp \
../src/device/value/channel_floating_point_sensor_value.cpp \
../src/device/value/channel_gate_value.cpp \
../src/device/value/channel_general_purpose_base_value.cpp \
../src/device/value/channel_general_purpose_measurement_value.cpp \
../src/device/value/channel_general_purpose_meter_value.cpp \
../src/device/value/channel_hp_thermostat_value.cpp \
../src/device/value/channel_hvac_value.cpp \
../src/device/value/channel_hvac_value_with_temphum.cpp \
../src/device/value/channel_ic_value.cpp \
../src/device/value/channel_onoff_value.cpp \
../src/device/value/channel_openclosed_value.cpp \
../src/device/value/channel_rgbw_value.cpp \
../src/device/value/channel_rs_value.cpp \
../src/device/value/channel_temphum_value.cpp \
../src/device/value/channel_value.cpp \
../src/device/value/channel_value_envelope.cpp \
../src/device/value/channel_value_factory.cpp \
../src/device/value/channel_valve_value.cpp 

CPP_DEPS += \
./src/device/value/abstract_channel_value.d \
./src/device/value/channel_binary_sensor_value.d \
./src/device/value/channel_container_value.d \
./src/device/value/channel_dgf_value.d \
./src/device/value/channel_em_value.d \
./src/device/value/channel_fb_value.d \
./src/device/value/channel_floating_point_sensor_value.d \
./src/device/value/channel_gate_value.d \
./src/device/value/channel_general_purpose_base_value.d \
./src/device/value/channel_general_purpose_measurement_value.d \
./src/device/value/channel_general_purpose_meter_value.d \
./src/device/value/channel_hp_thermostat_value.d \
./src/device/value/channel_hvac_value.d \
./src/device/value/channel_hvac_value_with_temphum.d \
./src/device/value/channel_ic_value.d \
./src/device/value/channel_onoff_value.d \
./src/device/value/channel_openclosed_value.d \
./src/device/value/channel_rgbw_value.d \
./src/device/value/channel_rs_value.d \
./src/device/value/channel_temphum_value.d \
./src/device/value/channel_value.d \
./src/device/value/channel_value_envelope.d \
./src/device/value/channel_value_factory.d \
./src/device/value/channel_valve_value.d 

OBJS += \
./src/device/value/abstract_channel_value.o \
./src/device/value/channel_binary_sensor_value.o \
./src/device/value/channel_container_value.o \
./src/device/value/channel_dgf_value.o \
./src/device/value/channel_em_value.o \
./src/device/value/channel_fb_value.o \
./src/device/value/channel_floating_point_sensor_value.o \
./src/device/value/channel_gate_value.o \
./src/device/value/channel_general_purpose_base_value.o \
./src/device/value/channel_general_purpose_measurement_value.o \
./src/device/value/channel_general_purpose_meter_value.o \
./src/device/value/channel_hp_thermostat_value.o \
./src/device/value/channel_hvac_value.o \
./src/device/value/channel_hvac_value_with_temphum.o \
./src/device/value/channel_ic_value.o \
./src/device/value/channel_onoff_value.o \
./src/device/value/channel_openclosed_value.o \
./src/device/value/channel_rgbw_value.o \
./src/device/value/channel_rs_value.o \
./src/device/value/channel_temphum_value.o \
./src/device/value/channel_value.o \
./src/device/value/channel_value_envelope.o \
./src/device/value/channel_value_factory.o \
./src/device/value/channel_valve_value.o 


# Each subdirectory must supply rules for building sources it contributes
src/device/value/%.o: ../src/device/value/%.cpp src/device/value/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O2 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-device-2f-value

clean-src-2f-device-2f-value:
	-$(RM) ./src/device/value/abstract_channel_value.d ./src/device/value/abstract_channel_value.o ./src/device/value/channel_binary_sensor_value.d ./src/device/value/channel_binary_sensor_value.o ./src/device/value/channel_container_value.d ./src/device/value/channel_container_value.o ./src/device/value/channel_dgf_value.d ./src/device/value/channel_dgf_value.o ./src/device/value/channel_em_value.d ./src/device/value/channel_em_value.o ./src/device/value/channel_fb_value.d ./src/device/value/channel_fb_value.o ./src/device/value/channel_floating_point_sensor_value.d ./src/device/value/channel_floating_point_sensor_value.o ./src/device/value/channel_gate_value.d ./src/device/value/channel_gate_value.o ./src/device/value/channel_general_purpose_base_value.d ./src/device/value/channel_general_purpose_base_value.o ./src/device/value/channel_general_purpose_measurement_value.d ./src/device/value/channel_general_purpose_measurement_value.o ./src/device/value/channel_general_purpose_meter_value.d ./src/device/value/channel_general_purpose_meter_value.o ./src/device/value/channel_hp_thermostat_value.d ./src/device/value/channel_hp_thermostat_value.o ./src/device/value/channel_hvac_value.d ./src/device/value/channel_hvac_value.o ./src/device/value/channel_hvac_value_with_temphum.d ./src/device/value/channel_hvac_value_with_temphum.o ./src/device/value/channel_ic_value.d ./src/device/value/channel_ic_value.o ./src/device/value/channel_onoff_value.d ./src/device/value/channel_onoff_value.o ./src/device/value/channel_openclosed_value.d ./src/device/value/channel_openclosed_value.o ./src/device/value/channel_rgbw_value.d ./src/device/value/channel_rgbw_value.o ./src/device/value/channel_rs_value.d ./src/device/value/channel_rs_value.o ./src/device/value/channel_temphum_value.d ./src/device/value/channel_temphum_value.o ./src/device/value/channel_value.d ./src/device/value/channel_value.o ./src/device/value/channel_value_envelope.d ./src/device/value/channel_value_envelope.o ./src/device/value/channel_value_factory.d ./src/device/value/channel_value_factory.o ./src/device/value/channel_valve_value.d ./src/device/value/channel_valve_value.o

.PHONY: clean-src-2f-device-2f-value

