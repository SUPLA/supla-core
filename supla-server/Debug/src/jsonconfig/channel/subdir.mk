################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/jsonconfig/channel/abstract_channel_json_config_getter.cpp \
../src/jsonconfig/channel/abstract_voice_assistant_config.cpp \
../src/jsonconfig/channel/action_trigger_config.cpp \
../src/jsonconfig/channel/alexa_config.cpp \
../src/jsonconfig/channel/channel_json_config.cpp \
../src/jsonconfig/channel/channel_json_config_getter.cpp \
../src/jsonconfig/channel/controlling_the_gate_config.cpp \
../src/jsonconfig/channel/electricity_meter_config.cpp \
../src/jsonconfig/channel/google_home_config.cpp \
../src/jsonconfig/channel/impulse_counter_config.cpp \
../src/jsonconfig/channel/opening_sensor_config.cpp 

CPP_DEPS += \
./src/jsonconfig/channel/abstract_channel_json_config_getter.d \
./src/jsonconfig/channel/abstract_voice_assistant_config.d \
./src/jsonconfig/channel/action_trigger_config.d \
./src/jsonconfig/channel/alexa_config.d \
./src/jsonconfig/channel/channel_json_config.d \
./src/jsonconfig/channel/channel_json_config_getter.d \
./src/jsonconfig/channel/controlling_the_gate_config.d \
./src/jsonconfig/channel/electricity_meter_config.d \
./src/jsonconfig/channel/google_home_config.d \
./src/jsonconfig/channel/impulse_counter_config.d \
./src/jsonconfig/channel/opening_sensor_config.d 

OBJS += \
./src/jsonconfig/channel/abstract_channel_json_config_getter.o \
./src/jsonconfig/channel/abstract_voice_assistant_config.o \
./src/jsonconfig/channel/action_trigger_config.o \
./src/jsonconfig/channel/alexa_config.o \
./src/jsonconfig/channel/channel_json_config.o \
./src/jsonconfig/channel/channel_json_config_getter.o \
./src/jsonconfig/channel/controlling_the_gate_config.o \
./src/jsonconfig/channel/electricity_meter_config.o \
./src/jsonconfig/channel/google_home_config.o \
./src/jsonconfig/channel/impulse_counter_config.o \
./src/jsonconfig/channel/opening_sensor_config.o 


# Each subdirectory must supply rules for building sources it contributes
src/jsonconfig/channel/%.o: ../src/jsonconfig/channel/%.cpp src/jsonconfig/channel/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-jsonconfig-2f-channel

clean-src-2f-jsonconfig-2f-channel:
	-$(RM) ./src/jsonconfig/channel/abstract_channel_json_config_getter.d ./src/jsonconfig/channel/abstract_channel_json_config_getter.o ./src/jsonconfig/channel/abstract_voice_assistant_config.d ./src/jsonconfig/channel/abstract_voice_assistant_config.o ./src/jsonconfig/channel/action_trigger_config.d ./src/jsonconfig/channel/action_trigger_config.o ./src/jsonconfig/channel/alexa_config.d ./src/jsonconfig/channel/alexa_config.o ./src/jsonconfig/channel/channel_json_config.d ./src/jsonconfig/channel/channel_json_config.o ./src/jsonconfig/channel/channel_json_config_getter.d ./src/jsonconfig/channel/channel_json_config_getter.o ./src/jsonconfig/channel/controlling_the_gate_config.d ./src/jsonconfig/channel/controlling_the_gate_config.o ./src/jsonconfig/channel/electricity_meter_config.d ./src/jsonconfig/channel/electricity_meter_config.o ./src/jsonconfig/channel/google_home_config.d ./src/jsonconfig/channel/google_home_config.o ./src/jsonconfig/channel/impulse_counter_config.d ./src/jsonconfig/channel/impulse_counter_config.o ./src/jsonconfig/channel/opening_sensor_config.d ./src/jsonconfig/channel/opening_sensor_config.o

.PHONY: clean-src-2f-jsonconfig-2f-channel

