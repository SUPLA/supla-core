################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/channeljsonconfig/abstract_channel_json_config_getter.cpp \
../src/channeljsonconfig/action_trigger_config.cpp \
../src/channeljsonconfig/channel_json_config.cpp \
../src/channeljsonconfig/channel_json_config_getter.cpp \
../src/channeljsonconfig/controlling_the_gate_config.cpp \
../src/channeljsonconfig/electricity_meter_config.cpp \
../src/channeljsonconfig/google_home_config.cpp \
../src/channeljsonconfig/impulse_counter_config.cpp \
../src/channeljsonconfig/opening_sensor_config.cpp 

CPP_DEPS += \
./src/channeljsonconfig/abstract_channel_json_config_getter.d \
./src/channeljsonconfig/action_trigger_config.d \
./src/channeljsonconfig/channel_json_config.d \
./src/channeljsonconfig/channel_json_config_getter.d \
./src/channeljsonconfig/controlling_the_gate_config.d \
./src/channeljsonconfig/electricity_meter_config.d \
./src/channeljsonconfig/google_home_config.d \
./src/channeljsonconfig/impulse_counter_config.d \
./src/channeljsonconfig/opening_sensor_config.d 

OBJS += \
./src/channeljsonconfig/abstract_channel_json_config_getter.o \
./src/channeljsonconfig/action_trigger_config.o \
./src/channeljsonconfig/channel_json_config.o \
./src/channeljsonconfig/channel_json_config_getter.o \
./src/channeljsonconfig/controlling_the_gate_config.o \
./src/channeljsonconfig/electricity_meter_config.o \
./src/channeljsonconfig/google_home_config.o \
./src/channeljsonconfig/impulse_counter_config.o \
./src/channeljsonconfig/opening_sensor_config.o 


# Each subdirectory must supply rules for building sources it contributes
src/channeljsonconfig/%.o: ../src/channeljsonconfig/%.cpp src/channeljsonconfig/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O2 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-channeljsonconfig

clean-src-2f-channeljsonconfig:
	-$(RM) ./src/channeljsonconfig/abstract_channel_json_config_getter.d ./src/channeljsonconfig/abstract_channel_json_config_getter.o ./src/channeljsonconfig/action_trigger_config.d ./src/channeljsonconfig/action_trigger_config.o ./src/channeljsonconfig/channel_json_config.d ./src/channeljsonconfig/channel_json_config.o ./src/channeljsonconfig/channel_json_config_getter.d ./src/channeljsonconfig/channel_json_config_getter.o ./src/channeljsonconfig/controlling_the_gate_config.d ./src/channeljsonconfig/controlling_the_gate_config.o ./src/channeljsonconfig/electricity_meter_config.d ./src/channeljsonconfig/electricity_meter_config.o ./src/channeljsonconfig/google_home_config.d ./src/channeljsonconfig/google_home_config.o ./src/channeljsonconfig/impulse_counter_config.d ./src/channeljsonconfig/impulse_counter_config.o ./src/channeljsonconfig/opening_sensor_config.d ./src/channeljsonconfig/opening_sensor_config.o

.PHONY: clean-src-2f-channeljsonconfig

