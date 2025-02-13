################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/jsonconfig/channel/abstract_voice_assistant_config.cpp \
../src/jsonconfig/channel/action_trigger_config.cpp \
../src/jsonconfig/channel/alexa_config.cpp \
../src/jsonconfig/channel/alt_weekly_schedule_config.cpp \
../src/jsonconfig/channel/binary_sensor_config.cpp \
../src/jsonconfig/channel/channel_json_config_getter.cpp \
../src/jsonconfig/channel/container_config.cpp \
../src/jsonconfig/channel/controlling_the_gate_config.cpp \
../src/jsonconfig/channel/electricity_meter_config.cpp \
../src/jsonconfig/channel/facade_blind_config.cpp \
../src/jsonconfig/channel/general_purpose_base_config.cpp \
../src/jsonconfig/channel/general_purpose_measurement_config.cpp \
../src/jsonconfig/channel/general_purpose_meter_config.cpp \
../src/jsonconfig/channel/google_home_config.cpp \
../src/jsonconfig/channel/hvac_config.cpp \
../src/jsonconfig/channel/impulse_counter_config.cpp \
../src/jsonconfig/channel/light_switch_config.cpp \
../src/jsonconfig/channel/ocr_config.cpp \
../src/jsonconfig/channel/power_switch_config.cpp \
../src/jsonconfig/channel/roller_shutter_config.cpp \
../src/jsonconfig/channel/temp_hum_config.cpp \
../src/jsonconfig/channel/valve_config.cpp \
../src/jsonconfig/channel/weekly_schedule_config.cpp 

CPP_DEPS += \
./src/jsonconfig/channel/abstract_voice_assistant_config.d \
./src/jsonconfig/channel/action_trigger_config.d \
./src/jsonconfig/channel/alexa_config.d \
./src/jsonconfig/channel/alt_weekly_schedule_config.d \
./src/jsonconfig/channel/binary_sensor_config.d \
./src/jsonconfig/channel/channel_json_config_getter.d \
./src/jsonconfig/channel/container_config.d \
./src/jsonconfig/channel/controlling_the_gate_config.d \
./src/jsonconfig/channel/electricity_meter_config.d \
./src/jsonconfig/channel/facade_blind_config.d \
./src/jsonconfig/channel/general_purpose_base_config.d \
./src/jsonconfig/channel/general_purpose_measurement_config.d \
./src/jsonconfig/channel/general_purpose_meter_config.d \
./src/jsonconfig/channel/google_home_config.d \
./src/jsonconfig/channel/hvac_config.d \
./src/jsonconfig/channel/impulse_counter_config.d \
./src/jsonconfig/channel/light_switch_config.d \
./src/jsonconfig/channel/ocr_config.d \
./src/jsonconfig/channel/power_switch_config.d \
./src/jsonconfig/channel/roller_shutter_config.d \
./src/jsonconfig/channel/temp_hum_config.d \
./src/jsonconfig/channel/valve_config.d \
./src/jsonconfig/channel/weekly_schedule_config.d 

OBJS += \
./src/jsonconfig/channel/abstract_voice_assistant_config.o \
./src/jsonconfig/channel/action_trigger_config.o \
./src/jsonconfig/channel/alexa_config.o \
./src/jsonconfig/channel/alt_weekly_schedule_config.o \
./src/jsonconfig/channel/binary_sensor_config.o \
./src/jsonconfig/channel/channel_json_config_getter.o \
./src/jsonconfig/channel/container_config.o \
./src/jsonconfig/channel/controlling_the_gate_config.o \
./src/jsonconfig/channel/electricity_meter_config.o \
./src/jsonconfig/channel/facade_blind_config.o \
./src/jsonconfig/channel/general_purpose_base_config.o \
./src/jsonconfig/channel/general_purpose_measurement_config.o \
./src/jsonconfig/channel/general_purpose_meter_config.o \
./src/jsonconfig/channel/google_home_config.o \
./src/jsonconfig/channel/hvac_config.o \
./src/jsonconfig/channel/impulse_counter_config.o \
./src/jsonconfig/channel/light_switch_config.o \
./src/jsonconfig/channel/ocr_config.o \
./src/jsonconfig/channel/power_switch_config.o \
./src/jsonconfig/channel/roller_shutter_config.o \
./src/jsonconfig/channel/temp_hum_config.o \
./src/jsonconfig/channel/valve_config.o \
./src/jsonconfig/channel/weekly_schedule_config.o 


# Each subdirectory must supply rules for building sources it contributes
src/jsonconfig/channel/%.o: ../src/jsonconfig/channel/%.cpp src/jsonconfig/channel/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O2 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-jsonconfig-2f-channel

clean-src-2f-jsonconfig-2f-channel:
	-$(RM) ./src/jsonconfig/channel/abstract_voice_assistant_config.d ./src/jsonconfig/channel/abstract_voice_assistant_config.o ./src/jsonconfig/channel/action_trigger_config.d ./src/jsonconfig/channel/action_trigger_config.o ./src/jsonconfig/channel/alexa_config.d ./src/jsonconfig/channel/alexa_config.o ./src/jsonconfig/channel/alt_weekly_schedule_config.d ./src/jsonconfig/channel/alt_weekly_schedule_config.o ./src/jsonconfig/channel/binary_sensor_config.d ./src/jsonconfig/channel/binary_sensor_config.o ./src/jsonconfig/channel/channel_json_config_getter.d ./src/jsonconfig/channel/channel_json_config_getter.o ./src/jsonconfig/channel/container_config.d ./src/jsonconfig/channel/container_config.o ./src/jsonconfig/channel/controlling_the_gate_config.d ./src/jsonconfig/channel/controlling_the_gate_config.o ./src/jsonconfig/channel/electricity_meter_config.d ./src/jsonconfig/channel/electricity_meter_config.o ./src/jsonconfig/channel/facade_blind_config.d ./src/jsonconfig/channel/facade_blind_config.o ./src/jsonconfig/channel/general_purpose_base_config.d ./src/jsonconfig/channel/general_purpose_base_config.o ./src/jsonconfig/channel/general_purpose_measurement_config.d ./src/jsonconfig/channel/general_purpose_measurement_config.o ./src/jsonconfig/channel/general_purpose_meter_config.d ./src/jsonconfig/channel/general_purpose_meter_config.o ./src/jsonconfig/channel/google_home_config.d ./src/jsonconfig/channel/google_home_config.o ./src/jsonconfig/channel/hvac_config.d ./src/jsonconfig/channel/hvac_config.o ./src/jsonconfig/channel/impulse_counter_config.d ./src/jsonconfig/channel/impulse_counter_config.o ./src/jsonconfig/channel/light_switch_config.d ./src/jsonconfig/channel/light_switch_config.o ./src/jsonconfig/channel/ocr_config.d ./src/jsonconfig/channel/ocr_config.o ./src/jsonconfig/channel/power_switch_config.d ./src/jsonconfig/channel/power_switch_config.o ./src/jsonconfig/channel/roller_shutter_config.d ./src/jsonconfig/channel/roller_shutter_config.o ./src/jsonconfig/channel/temp_hum_config.d ./src/jsonconfig/channel/temp_hum_config.o ./src/jsonconfig/channel/valve_config.d ./src/jsonconfig/channel/valve_config.o ./src/jsonconfig/channel/weekly_schedule_config.d ./src/jsonconfig/channel/weekly_schedule_config.o

.PHONY: clean-src-2f-jsonconfig-2f-channel

